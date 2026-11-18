#pragma once

#include <string>
#include <optional>
#include "models/Models.hpp"

namespace attendance::auth {

struct SessionClaims {
    std::string userId;
    std::string email;
    std::string name;
    attendance::models::UserRole role;
    std::string studentId;
    std::string facultyId;
};

class AuthService {
public:
    static std::string hashPassword(const std::string& plainPassword);
    static bool verifyPassword(const std::string& plainPassword, const std::string& storedHash);
    static std::string generateToken(const SessionClaims& claims, const std::string& secretKey);
    static std::optional<SessionClaims> verifyToken(const std::string& token, const std::string& secretKey);
};

} // namespace attendance::auth
