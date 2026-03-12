#include "auth_handler.hpp"
#include <random>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <openssl/sha.h>

AuthHandler::AuthHandler() {}

std::string AuthHandler::hashPassword(const std::string& password) {
    // Simple SHA256 hash (in production, use bcrypt or argon2)
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password.c_str(), password.length(), hash);
    
    std::stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

bool AuthHandler::verifyPassword(const std::string& password, const std::string& hash) {
    return hashPassword(password) == hash;
}

std::string AuthHandler::generateRandomToken() {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(alphanum) - 2);
    
    std::string token;
    for (int i = 0; i < 32; i++) {
        token += alphanum[dis(gen)];
    }
    return token;
}

bool AuthHandler::registerUser(const std::string& username, const std::string& email, const std::string& password) {
    // Check if user already exists
    if (users.find(username) != users.end()) {
        return false;
    }
    
    // Create new user
    User user;
    user.id = users.size() + 1;
    user.username = username;
    user.email = email;
    user.password_hash = hashPassword(password);
    
    users[username] = user;
    return true;
}

bool AuthHandler::loginUser(const std::string& username, const std::string& password) {
    auto it = users.find(username);
    if (it == users.end()) {
        return false;
    }
    
    return verifyPassword(password, it->second.password_hash);
}

std::string AuthHandler::generateToken(const std::string& username) {
    std::string token = generateRandomToken();
    tokens[token] = username;
    return token;
}

bool AuthHandler::validateToken(const std::string& token) {
    return tokens.find(token) != tokens.end();
}

std::string AuthHandler::getUsernameFromToken(const std::string& token) {
    auto it = tokens.find(token);
    if (it != tokens.end()) {
        return it->second;
    }
    return "";
}
