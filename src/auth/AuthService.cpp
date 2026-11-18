#include "auth/AuthService.hpp"
#include <sstream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unordered_map>
#include <mutex>
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
#include "monocypher.h"
#ifdef __cplusplus
}
#endif

namespace attendance::auth {

// Thread-safe in-memory login rate limiter
struct RateLimitEntry {
    int attempts{0};
    long long firstAttemptEpoch{0};
};

static std::unordered_map<std::string, RateLimitEntry> g_rateLimitMap;
static std::mutex g_rateLimitMutex;

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
        return "";
    }
    std::stringstream ss;
    for (uint8_t b : buffer) {
        ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(b);
    }
    return ss.str();
}

std::string AuthService::generateSessionToken() {
    return generateSecureToken(32); // 64 hex characters
}

std::string AuthService::hashSessionToken(const std::string& rawToken) {
    SHA256_CTX ctx;
    uint8_t hash[32];
    sha256_init(&ctx);
    sha256_update(&ctx, reinterpret_cast<const uint8_t*>(rawToken.data()), rawToken.length());
    sha256_final(&ctx, hash);

    std::stringstream ss;
    for (int i = 0; i < 32; i++) {
        ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(hash[i]);
    }
    return ss.str();
}

std::string AuthService::generateCsrfToken(const std::string& sessionToken) {
    std::string secret = getJwtSecret();
    std::string combined = sessionToken + ":csrf:" + secret;
    return hashSessionToken(combined);
}

bool AuthService::verifyCsrfToken(const std::string& sessionToken, const std::string& providedToken) {
    if (sessionToken.empty() || providedToken.empty()) return false;
    std::string expected = generateCsrfToken(sessionToken);
    return constantTimeEquals(expected, providedToken);
}

bool AuthService::isRateLimited(const std::string& key, int maxAttempts, int windowSeconds) {
    std::lock_guard<std::mutex> lock(g_rateLimitMutex);
    auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    auto it = g_rateLimitMap.find(key);
    if (it == g_rateLimitMap.end()) return false;

    if (now - it->second.firstAttemptEpoch > windowSeconds) {
        // Window expired, reset
        g_rateLimitMap.erase(it);
        return false;
    }

    return it->second.attempts >= maxAttempts;
}

void AuthService::recordAttempt(const std::string& key, bool success) {
    std::lock_guard<std::mutex> lock(g_rateLimitMutex);
    auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    if (success) {
        g_rateLimitMap.erase(key);
        return;
    }

    auto& entry = g_rateLimitMap[key];
    if (entry.attempts == 0 || (now - entry.firstAttemptEpoch > 300)) {
        entry.attempts = 1;
        entry.firstAttemptEpoch = now;
    } else {
        entry.attempts++;
    }
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
    return secret.length() >= 32;
}

std::string AuthService::getJwtSecret() {
    const char* envSecret = std::getenv("JWT_SECRET");
    if (envSecret && std::strlen(envSecret) >= 32) {
        return std::string(envSecret);
    }
    return "";
}

// -------------------------------------------------------------
// Argon2id Password Hashing (via Monocypher)
// -------------------------------------------------------------
std::string AuthService::hashPasswordArgon2id(const std::string& plainPassword) {
    uint8_t salt[16];
    if (!getSecureRandomBytes(salt, 16)) {
        return "";
    }

    crypto_argon2_config config;
    config.algorithm = CRYPTO_ARGON2_ID;
    config.nb_blocks = 64; // 64 KiB memory hardness
    config.nb_passes = 2;  // 2 iterations
    config.nb_lanes  = 1;  // 1 lane

    crypto_argon2_inputs inputs;
    inputs.pass = reinterpret_cast<const uint8_t*>(plainPassword.data());
    inputs.pass_size = static_cast<uint32_t>(plainPassword.size());
    inputs.salt = salt;
    inputs.salt_size = 16;

    std::vector<uint8_t> workArea(config.nb_blocks * 1024);
    uint8_t hashOut[32];

    crypto_argon2(hashOut, 32, workArea.data(), config, inputs, crypto_argon2_no_extras);

    std::stringstream ss;
    ss << "$argon2id$v=19$m=64,t=2,p=1$";
    for (int i = 0; i < 16; i++) {
        ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(salt[i]);
    }
    ss << "$";
    for (int i = 0; i < 32; i++) {
        ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(hashOut[i]);
    }
    return ss.str();
}

std::string AuthService::hashPasswordBcrypt(const std::string& plainPassword) {
    char saltBytes[16];
    if (!getSecureRandomBytes(saltBytes, 16)) {
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

std::string AuthService::hashPassword(const std::string& plainPassword) {
    // Default to Argon2id for new password hashes
    std::string h = hashPasswordArgon2id(plainPassword);
    if (!h.empty()) return h;
    return hashPasswordBcrypt(plainPassword);
}

bool AuthService::verifyPassword(const std::string& plainPassword, const std::string& storedHash) {
    if (plainPassword.empty() || storedHash.empty()) return false;

    if (isSetupRequired(storedHash)) {
        return false;
    }

    // 1. Check Argon2id hash format: $argon2id$v=19$m=64,t=2,p=1$<salt_hex>$<hash_hex>
    if (storedHash.rfind("$argon2id$", 0) == 0) {
        size_t lastDollar = storedHash.rfind('$');
        if (lastDollar == std::string::npos) return false;
        size_t secondLastDollar = storedHash.rfind('$', lastDollar - 1);
        if (secondLastDollar == std::string::npos) return false;

        std::string saltHex = storedHash.substr(secondLastDollar + 1, lastDollar - secondLastDollar - 1);
        std::string expectedHashHex = storedHash.substr(lastDollar + 1);

        if (saltHex.length() != 32 || expectedHashHex.length() != 64) return false;

        uint8_t salt[16];
        for (size_t i = 0; i < 16; i++) {
            salt[i] = static_cast<uint8_t>(std::stoul(saltHex.substr(i * 2, 2), nullptr, 16));
        }

        crypto_argon2_config config;
        config.algorithm = CRYPTO_ARGON2_ID;
        config.nb_blocks = 64;
        config.nb_passes = 2;
        config.nb_lanes  = 1;

        crypto_argon2_inputs inputs;
        inputs.pass = reinterpret_cast<const uint8_t*>(plainPassword.data());
        inputs.pass_size = static_cast<uint32_t>(plainPassword.size());
        inputs.salt = salt;
        inputs.salt_size = 16;

        std::vector<uint8_t> workArea(config.nb_blocks * 1024);
        uint8_t hashOut[32];
        crypto_argon2(hashOut, 32, workArea.data(), config, inputs, crypto_argon2_no_extras);

        std::stringstream ss;
        for (int i = 0; i < 32; i++) {
            ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(hashOut[i]);
        }

        return constantTimeEquals(ss.str(), expectedHashHex);
    }

    // 2. Backward compatibility: Standard bcrypt hash ($2a$ or $2b$)
    if (storedHash.rfind("$2a$", 0) == 0 || storedHash.rfind("$2b$", 0) == 0) {
        char hashOut[128] = {0};
        char* res = _crypt_blowfish_rn(plainPassword.c_str(), storedHash.c_str(), hashOut, sizeof(hashOut));
        if (!res) return false;
        return constantTimeEquals(std::string(hashOut), storedHash);
    }

    return false;
}

bool AuthService::isSetupRequired(const std::string& storedHash) {
    return storedHash.rfind("!SETUP_REQUIRED:", 0) == 0;
}

bool AuthService::verifySetupToken(const std::string& storedHash, const std::string& providedToken) {
    if (!isSetupRequired(storedHash) || providedToken.empty()) return false;

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
        return false;
    }

    return constantTimeEquals(expectedToken, providedToken);
}

// -------------------------------------------------------------
// JWT Support for API Interoperability
// -------------------------------------------------------------
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

static std::string base64UrlEncode(const std::string& input) {
    static const char* b64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    std::string output;
    int val = 0, valb = -6;
    for (unsigned char c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            output.push_back(b64Chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        output.push_back(b64Chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    return output;
}

static std::string base64UrlDecode(const std::string& input) {
    if (!isValidBase64Url(input)) return "";
    std::string output;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) {
        T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"[i]] = i;
    }

    int val = 0, valb = -8;
    for (unsigned char c : input) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            output.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return output;
}

static std::string hmacSha256(const std::string& key, const std::string& data) {
    uint8_t k[64] = {0};
    if (key.length() > 64) {
        SHA256_CTX ctx;
        sha256_init(&ctx);
        sha256_update(&ctx, reinterpret_cast<const uint8_t*>(key.data()), key.length());
        sha256_final(&ctx, k);
    } else {
        std::memcpy(k, key.data(), key.length());
    }

    uint8_t k_ipad[64];
    uint8_t k_opad[64];
    for (int i = 0; i < 64; i++) {
        k_ipad[i] = k[i] ^ 0x36;
        k_opad[i] = k[i] ^ 0x5c;
    }

    uint8_t innerHash[32];
    SHA256_CTX innerCtx;
    sha256_init(&innerCtx);
    sha256_update(&innerCtx, k_ipad, 64);
    sha256_update(&innerCtx, reinterpret_cast<const uint8_t*>(data.data()), data.length());
    sha256_final(&innerCtx, innerHash);

    uint8_t outerHash[32];
    SHA256_CTX outerCtx;
    sha256_init(&outerCtx);
    sha256_update(&outerCtx, k_opad, 64);
    sha256_update(&outerCtx, innerHash, 32);
    sha256_final(&outerCtx, outerHash);

    return std::string(reinterpret_cast<char*>(outerHash), 32);
}

std::string AuthService::generateToken(const SessionClaims& claims, const std::string& secretKey) {
    std::string secret = secretKey.empty() ? getJwtSecret() : secretKey;
    if (!validateSecret(secret)) return "";

    auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    long long iat = (claims.issuedAt > 0) ? claims.issuedAt : now;
    long long exp = (claims.expiresAt > 0) ? claims.expiresAt : (now + 7 * 86400);

    nlohmann::json header = {
        {"alg", "HS256"},
        {"typ", "JWT"}
    };

    nlohmann::json payload = {
        {"sub", claims.userId},
        {"email", claims.email},
        {"name", claims.name},
        {"role", models::roleToString(claims.role)},
        {"studentId", claims.studentId},
        {"facultyId", claims.facultyId},
        {"iat", iat},
        {"exp", exp},
        {"iss", "attendance-risk-predictor"},
        {"aud", "attendance-risk-client"}
    };

    std::string encodedHeader = base64UrlEncode(header.dump());
    std::string encodedPayload = base64UrlEncode(payload.dump());
    std::string signingInput = encodedHeader + "." + encodedPayload;

    std::string rawSignature = hmacSha256(secret, signingInput);
    std::string encodedSignature = base64UrlEncode(rawSignature);

    return signingInput + "." + encodedSignature;
}

std::optional<SessionClaims> AuthService::verifyToken(const std::string& token, const std::string& secretKey) {
    std::string secret = secretKey.empty() ? getJwtSecret() : secretKey;
    if (!validateSecret(secret)) return std::nullopt;

    size_t firstDot = token.find('.');
    size_t lastDot = token.rfind('.');
    if (firstDot == std::string::npos || lastDot == std::string::npos || firstDot == lastDot) {
        return std::nullopt;
    }

    std::string encodedHeader = token.substr(0, firstDot);
    std::string encodedPayload = token.substr(firstDot + 1, lastDot - firstDot - 1);
    std::string encodedSignature = token.substr(lastDot + 1);

    if (!isValidBase64Url(encodedHeader) || !isValidBase64Url(encodedPayload) || !isValidBase64Url(encodedSignature)) {
        return std::nullopt;
    }

    std::string signingInput = encodedHeader + "." + encodedPayload;
    std::string expectedSignature = base64UrlEncode(hmacSha256(secret, signingInput));

    if (!constantTimeEquals(expectedSignature, encodedSignature)) {
        return std::nullopt;
    }

    try {
        auto headerJson = nlohmann::json::parse(base64UrlDecode(encodedHeader));
        if (headerJson.value("alg", "") != "HS256" || headerJson.value("typ", "") != "JWT") {
            return std::nullopt;
        }

        auto payloadJson = nlohmann::json::parse(base64UrlDecode(encodedPayload));
        if (payloadJson.value("iss", "") != "attendance-risk-predictor" ||
            payloadJson.value("aud", "") != "attendance-risk-client") {
            return std::nullopt;
        }

        if (!payloadJson.contains("exp") || !payloadJson.contains("iat") || !payloadJson.contains("sub")) {
            return std::nullopt;
        }

        long long exp = payloadJson["exp"].get<long long>();
        long long iat = payloadJson["iat"].get<long long>();

        auto now = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();

        if (now >= exp || iat <= 0) {
            return std::nullopt;
        }

        SessionClaims claims;
        claims.userId = payloadJson.value("sub", "");
        claims.email = payloadJson.value("email", "");
        claims.name = payloadJson.value("name", "");
        claims.role = models::stringToRole(payloadJson.value("role", "STUDENT"));
        claims.studentId = payloadJson.value("studentId", "");
        claims.facultyId = payloadJson.value("facultyId", "");
        claims.issuedAt = iat;
        claims.expiresAt = exp;

        return claims;
    } catch (...) {
        return std::nullopt;
    }
}

} // namespace attendance::auth
