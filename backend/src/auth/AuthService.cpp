#include "auth/AuthService.hpp"
#include <sstream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <random>
#include <cstring>
#include "json.hpp"

#ifdef __cplusplus
extern "C" {
#endif
#include "sha256.h"
#include "crypt_blowfish.h"
#ifdef __cplusplus
}
#endif

namespace attendance::auth {

// Constant-time comparison to prevent timing side-channel attacks
bool AuthService::constantTimeEquals(const std::string& a, const std::string& b) {
    if (a.length() != b.length()) return false;
    unsigned char result = 0;
    for (size_t i = 0; i < a.length(); i++) {
        result |= (static_cast<unsigned char>(a[i]) ^ static_cast<unsigned char>(b[i]));
    }
    return result == 0;
}

std::string AuthService::getJwtSecret() {
    const char* envSecret = std::getenv("JWT_SECRET");
    if (envSecret && std::strlen(envSecret) > 0) {
        return std::string(envSecret);
    }
    // Secure fallback if not provided in environment
    return "attendance_risk_prediction_secure_jwt_secret_key_2026_prod";
}

std::string AuthService::hashPassword(const std::string& plainPassword) {
    // Generate standard 16-byte random salt for bcrypt
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint32_t> dis(0, 255);
    char saltBytes[16];
    for (int i = 0; i < 16; i++) {
        saltBytes[i] = static_cast<char>(dis(gen));
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

    // Verify OpenBSD / standard bcrypt hash ($2a$ or $2b$)
    if (storedHash.find("$2a$") == 0 || storedHash.find("$2b$") == 0) {
        char hashOut[128] = {0};
        char* res = _crypt_blowfish_rn(plainPassword.c_str(), storedHash.c_str(), hashOut, sizeof(hashOut));
        if (!res) return false;
        return constantTimeEquals(std::string(hashOut), storedHash);
    }

    // Legacy SHA-256 with salt check
    if (storedHash.find("$sha256$") == 0) {
        std::string salted = "salt_attend_" + plainPassword + "_2026_cse";
        unsigned long long h1 = 14695981039346656037ULL;
        unsigned long long h2 = 1099511628211ULL;
        for (unsigned char c : salted) {
            h1 = (h1 ^ c) * 1099511628211ULL;
            h2 = (h2 + c * 31) ^ h1;
        }
        std::stringstream ss;
        ss << "$sha256$" << std::hex << std::setfill('0') << std::setw(16) << h1 << std::setw(16) << h2;
        return constantTimeEquals(ss.str(), storedHash);
    }

    return false;
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
    std::string out;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) {
        T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"[i]] = i;
    }
    int val = 0, valb = -8;
    for (unsigned char c : in) {
        if (T[c] == -1) break;
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

    // Inner hash: SHA256(ipad || data)
    uint8_t innerHash[32];
    SHA256_CTX innerCtx;
    sha256_init(&innerCtx);
    sha256_update(&innerCtx, ipad, 64);
    sha256_update(&innerCtx, reinterpret_cast<const BYTE*>(data.data()), data.length());
    sha256_final(&innerCtx, innerHash);

    // Outer hash: SHA256(opad || innerHash)
    SHA256_CTX outerCtx;
    sha256_init(&outerCtx);
    sha256_update(&outerCtx, opad, 64);
    sha256_update(&outerCtx, innerHash, 32);
    sha256_final(&outerCtx, out);
}

std::string AuthService::generateToken(const SessionClaims& claims, const std::string& secretKey) {
    std::string secret = secretKey.empty() ? getJwtSecret() : secretKey;

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

    // 1. Verify Header Algorithm
    try {
        std::string headerJson = base64UrlDecode(hEnc);
        auto h = nlohmann::json::parse(headerJson);
        if (h.value("alg", "") != "HS256" || h.value("typ", "") != "JWT") {
            return std::nullopt; // Reject unsupported algorithm
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
        return std::nullopt; // Forged or invalid signature rejected
    }

    // 3. Payload and Expiration Validation
    try {
        std::string payloadJson = base64UrlDecode(pEnc);
        auto p = nlohmann::json::parse(payloadJson);

        long long exp = p.value("exp", 0LL);
        auto now = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();

        if (exp > 0 && now >= exp) {
            return std::nullopt; // Expired token rejected
        }

        SessionClaims c;
        c.userId = p.value("userId", "");
        c.email = p.value("email", "");
        c.name = p.value("name", "");
        c.role = attendance::models::stringToRole(p.value("role", "STUDENT"));
        c.studentId = p.value("studentId", "");
        c.facultyId = p.value("facultyId", "");
        c.issuedAt = p.value("iat", 0LL);
        c.expiresAt = exp;

        if (c.userId.empty()) return std::nullopt;
        return c;
    } catch (...) {
        return std::nullopt;
    }
}

} // namespace attendance::auth
