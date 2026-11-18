#include "auth/AuthService.hpp"
#include <sstream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "json.hpp"

#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#else
#include <sys/random.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
#include "sha256.h"
#include "crypt_blowfish.h"
#ifdef __cplusplus
}
#endif

namespace attendance::auth {

// Cryptographically secure operating system random bytes generator
bool AuthService::getSecureRandomBytes(void* buffer, size_t length) {
    if (!buffer || length == 0) return false;

#ifdef _WIN32
    NTSTATUS status = BCryptGenRandom(
        NULL,
        static_cast<PUCHAR>(buffer),
        static_cast<ULONG>(length),
        BCRYPT_USE_SYSTEM_PREFERRED_RNG
    );
    return BCRYPT_SUCCESS(status);
#else
    ssize_t ret = getrandom(buffer, length, 0);
    if (ret == static_cast<ssize_t>(length)) return true;

    // Fallback to /dev/urandom on POSIX
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return false;
    size_t total = 0;
    while (total < length) {
        ssize_t r = read(fd, static_cast<char*>(buffer) + total, length - total);
        if (r <= 0) {
            close(fd);
            return false;
        }
        total += static_cast<size_t>(r);
    }
    close(fd);
    return true;
#endif
}

std::string AuthService::generateSecureToken(size_t byteCount) {
    std::vector<uint8_t> buffer(byteCount);
    if (!getSecureRandomBytes(buffer.data(), byteCount)) {
        // Fallback should never happen on modern OS
        return "";
    }
    std::stringstream ss;
    for (uint8_t b : buffer) {
        ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(b);
    }
    return ss.str();
}

bool AuthService::constantTimeEquals(const std::string& a, const std::string& b) {
    if (a.length() != b.length()) return false;
    unsigned char result = 0;
    for (size_t i = 0; i < a.length(); i++) {
        result |= (static_cast<unsigned char>(a[i]) ^ static_cast<unsigned char>(b[i]));
    }
    return result == 0;
}

bool AuthService::validateSecret(const std::string& secret) {
    // Requires at least 256 bits (32 bytes) of cryptographic material
    return secret.length() >= 32;
}

std::string AuthService::getJwtSecret() {
    const char* envSecret = std::getenv("JWT_SECRET");
    if (envSecret && std::strlen(envSecret) >= 32) {
        return std::string(envSecret);
    }
    // Hardcoded fallback completely eliminated
    return "";
}

std::string AuthService::hashPassword(const std::string& plainPassword) {
    char saltBytes[16];
    if (!getSecureRandomBytes(saltBytes, 16)) {
        std::cerr << "AuthService error: Cryptographic RNG failure for salt generation." << std::endl;
        return "";
    }

    char setting[64];
    if (!_crypt_gensalt_blowfish_rn("$2a$", 10, saltBytes, 16, setting, sizeof(setting))) {
        return "";
    }

    char hashOut[128];
    char* res = _crypt_blowfish_rn(plainPassword.c_str(), setting, hashOut, sizeof(hashOut));
    if (!res) return "";
    return std::string(hashOut);
}

bool AuthService::verifyPassword(const std::string& plainPassword, const std::string& storedHash) {
    if (plainPassword.empty() || storedHash.empty()) return false;

    // If account has not completed initial setup, refuse normal login
    if (isSetupRequired(storedHash)) {
        return false;
    }

    // Verify OpenBSD / standard bcrypt hash ($2a$ or $2b$)
    if (storedHash.rfind("$2a$", 0) == 0 || storedHash.rfind("$2b$", 0) == 0) {
        char hashOut[128] = {0};
        char* res = _crypt_blowfish_rn(plainPassword.c_str(), storedHash.c_str(), hashOut, sizeof(hashOut));
        if (!res) return false;
        return constantTimeEquals(std::string(hashOut), storedHash);
    }

    // Custom $sha256$ legacy password verification removed entirely
    return false;
}

bool AuthService::isSetupRequired(const std::string& storedHash) {
    return storedHash.rfind("!SETUP_REQUIRED:", 0) == 0;
}

bool AuthService::verifySetupToken(const std::string& storedHash, const std::string& providedToken) {
    if (!isSetupRequired(storedHash) || providedToken.empty()) return false;

    // Expected format: !SETUP_REQUIRED:<token>:<expiryEpoch>
    size_t firstColon = storedHash.find(':', 16);
    if (firstColon == std::string::npos) {
        std::string expectedToken = storedHash.substr(16);
        return constantTimeEquals(expectedToken, providedToken);
    }

    std::string expectedToken = storedHash.substr(16, firstColon - 16);
    std::string expStr = storedHash.substr(firstColon + 1);

    long long expEpoch = 0;
    try {
        expEpoch = std::stoll(expStr);
    } catch (...) {
        return false;
    }

    auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    if (now >= expEpoch) {
        return false; // Token expired
    }

    return constantTimeEquals(expectedToken, providedToken);
}

static bool isValidBase64Url(const std::string& str) {
    if (str.empty()) return false;
    for (char c : str) {
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || 
              (c >= '0' && c <= '9') || c == '-' || c == '_')) {
            return false;
        }
    }
    return true;
}

static std::string base64UrlEncode(const uint8_t* data, size_t len) {
    static const char tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    std::string out;
    int val = 0, valb = -6;
    for (size_t i = 0; i < len; i++) {
        val = (val << 8) + data[i];
        valb += 8;
        while (valb >= 0) {
            out.push_back(tbl[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(tbl[((val << 8) >> (valb + 8)) & 0x3F]);
    return out;
}

static std::string base64UrlEncodeStr(const std::string& str) {
    return base64UrlEncode(reinterpret_cast<const uint8_t*>(str.data()), str.length());
}

static std::string base64UrlDecode(const std::string& in) {
    if (!isValidBase64Url(in)) return "";
    std::string out;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) {
        T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"[i]] = i;
    }
    int val = 0, valb = -8;
    for (unsigned char c : in) {
        if (T[c] == -1) return "";
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

static void computeHmacSha256(const std::string& key, const std::string& data, uint8_t out[32]) {
    uint8_t k[64] = {0};
    if (key.length() > 64) {
        SHA256_CTX ctx;
        sha256_init(&ctx);
        sha256_update(&ctx, reinterpret_cast<const BYTE*>(key.data()), key.length());
        sha256_final(&ctx, k);
    } else {
        std::memcpy(k, key.data(), key.length());
    }

    uint8_t ipad[64];
    uint8_t opad[64];
    for (int i = 0; i < 64; i++) {
        ipad[i] = k[i] ^ 0x36;
        opad[i] = k[i] ^ 0x5c;
    }

    uint8_t innerHash[32];
    SHA256_CTX innerCtx;
    sha256_init(&innerCtx);
    sha256_update(&innerCtx, ipad, 64);
    sha256_update(&innerCtx, reinterpret_cast<const BYTE*>(data.data()), data.length());
    sha256_final(&innerCtx, innerHash);

    SHA256_CTX outerCtx;
    sha256_init(&outerCtx);
    sha256_update(&outerCtx, opad, 64);
    sha256_update(&outerCtx, innerHash, 32);
    sha256_final(&outerCtx, out);
}

std::string AuthService::generateToken(const SessionClaims& claims, const std::string& secretKey) {
    std::string secret = secretKey.empty() ? getJwtSecret() : secretKey;
    if (secret.empty() || !validateSecret(secret)) {
        return "";
    }

    nlohmann::json header = {
        {"alg", "HS256"},
        {"typ", "JWT"}
    };

    auto now = std::chrono::system_clock::now();
    long long iatSeconds = claims.issuedAt > 0 
        ? claims.issuedAt 
        : std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    long long expSeconds = claims.expiresAt > 0 
        ? claims.expiresAt 
        : iatSeconds + (7 * 24 * 3600); // 7 days

    nlohmann::json payload = {
        {"iss", "attendance-risk-predictor"},
        {"aud", "attendance-risk-client"},
        {"userId", claims.userId},
        {"email", claims.email},
        {"name", claims.name},
        {"role", attendance::models::roleToString(claims.role)},
        {"studentId", claims.studentId},
        {"facultyId", claims.facultyId},
        {"iat", iatSeconds},
        {"exp", expSeconds}
    };

    std::string hEnc = base64UrlEncodeStr(header.dump());
    std::string pEnc = base64UrlEncodeStr(payload.dump());
    std::string unsignedToken = hEnc + "." + pEnc;

    uint8_t sigRaw[32];
    computeHmacSha256(secret, unsignedToken, sigRaw);
    std::string sEnc = base64UrlEncode(sigRaw, 32);

    return unsignedToken + "." + sEnc;
}

std::optional<SessionClaims> AuthService::verifyToken(const std::string& token, const std::string& secretKey) {
    if (token.empty()) return std::nullopt;
    std::string secret = secretKey.empty() ? getJwtSecret() : secretKey;
    if (secret.empty() || !validateSecret(secret)) {
        return std::nullopt;
    }

    size_t d1 = token.find('.');
    size_t d2 = token.rfind('.');
    if (d1 == std::string::npos || d2 == std::string::npos || d1 == d2) {
        return std::nullopt;
    }

    std::string hEnc = token.substr(0, d1);
    std::string pEnc = token.substr(d1 + 1, d2 - d1 - 1);
    std::string sEnc = token.substr(d2 + 1);

    if (hEnc.empty() || pEnc.empty() || sEnc.empty()) {
        return std::nullopt;
    }

    if (!isValidBase64Url(hEnc) || !isValidBase64Url(pEnc) || !isValidBase64Url(sEnc)) {
        return std::nullopt;
    }

    // 1. Verify Header Algorithm
    try {
        std::string headerJson = base64UrlDecode(hEnc);
        if (headerJson.empty()) return std::nullopt;
        auto h = nlohmann::json::parse(headerJson);
        if (h.value("alg", "") != "HS256" || h.value("typ", "") != "JWT") {
            return std::nullopt;
        }
    } catch (...) {
        return std::nullopt;
    }

    // 2. Cryptographic Signature Verification
    std::string unsignedToken = hEnc + "." + pEnc;
    uint8_t expectedSigRaw[32];
    computeHmacSha256(secret, unsignedToken, expectedSigRaw);
    std::string expectedSigEnc = base64UrlEncode(expectedSigRaw, 32);

    if (!constantTimeEquals(sEnc, expectedSigEnc)) {
        return std::nullopt;
    }

    // 3. Payload and Claims Validation
    try {
        std::string payloadJson = base64UrlDecode(pEnc);
        if (payloadJson.empty()) return std::nullopt;
        auto p = nlohmann::json::parse(payloadJson);

        // Validate Issuer and Audience
        if (p.value("iss", "") != "attendance-risk-predictor" ||
            p.value("aud", "") != "attendance-risk-client") {
            return std::nullopt;
        }

        // Validate exp claim (Must be present, positive, and in the future)
        if (!p.contains("exp") || !p["exp"].is_number_integer()) {
            return std::nullopt;
        }
        long long exp = p["exp"].get<long long>();
        auto now = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();

        if (exp <= 0 || now >= exp) {
            return std::nullopt;
        }

        // Validate iat claim (Must be present, positive, and not in far future)
        if (!p.contains("iat") || !p["iat"].is_number_integer()) {
            return std::nullopt;
        }
        long long iat = p["iat"].get<long long>();
        if (iat <= 0 || iat > now + 300) {
            return std::nullopt;
        }

        SessionClaims c;
        c.userId = p.value("userId", "");
        c.email = p.value("email", "");
        c.name = p.value("name", "");
        c.role = attendance::models::stringToRole(p.value("role", "STUDENT"));
        c.studentId = p.value("studentId", "");
        c.facultyId = p.value("facultyId", "");
        c.issuedAt = iat;
        c.expiresAt = exp;

        if (c.userId.empty() || c.email.empty()) {
            return std::nullopt;
        }
        return c;
    } catch (...) {
        return std::nullopt;
    }
}

} // namespace attendance::auth
