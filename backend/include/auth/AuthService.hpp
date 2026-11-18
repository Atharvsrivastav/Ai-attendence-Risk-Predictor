#pragma once

#include <string>
#include <optional>
#include "models/Models.hpp"

namespace attendance::auth {

struct SessionClaims {
    std::string userId;
    std::string email;
    std::string name;
    attendance::models::UserRole role{attendance::models::UserRole::STUDENT};
    std::string studentId;
    std::string facultyId;
    long long issuedAt{0};
    long long expiresAt{0};
};

class AuthService {
public:
    // Password Hashing and Verification
    static std::string hashPassword(const std::string& plainPassword);
    static bool verifyPassword(const std::string& plainPassword, const std::string& storedHash);

    // Cryptographic JWT (HS256)
    static std::string generateToken(const SessionClaims& claims, const std::string& secretKey = "");
    static std::optional<SessionClaims> verifyToken(const std::string& token, const std::string& secretKey = "");

    // Secret Management
    static std::string getJwtSecret();
    
    // Constant-time string comparison to prevent timing attacks
    static bool constantTimeEquals(const std::string& a, const std::string& b);
};

} // namespace attendance::auth
