#ifndef AUTH_HANDLER_HPP
#define AUTH_HANDLER_HPP

#include <string>
#include <map>

struct User {
    int id;
    std::string username;
    std::string email;
    std::string password_hash;
};

class AuthHandler {
public:
    AuthHandler();
    
    // Register new user
    bool registerUser(const std::string& username, const std::string& email, const std::string& password);
    
    // Login user
    bool loginUser(const std::string& username, const std::string& password);
    
    // Validate session token
    bool validateToken(const std::string& token);
    
    // Generate session token
    std::string generateToken(const std::string& username);
    
    // Get user by token
    std::string getUsernameFromToken(const std::string& token);
    
private:
    std::map<std::string, User> users; // In production, use database
    std::map<std::string, std::string> tokens; // token -> username
    
    std::string hashPassword(const std::string& password);
    bool verifyPassword(const std::string& password, const std::string& hash);
    std::string generateRandomToken();
};

#endif // AUTH_HANDLER_HPP