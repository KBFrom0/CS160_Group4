#include "User.h"

User::User() : username(""), nickname(""), salt(""), passwordHash("") {}

User::User(const std::string& username, const std::string& nickname,
           const std::string& salt, const std::string& passwordHash)
    : username(username), nickname(nickname), salt(salt), passwordHash(passwordHash) {}
