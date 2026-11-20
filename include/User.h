#ifndef USER_H
#define USER_H

#include <string>

// One account of the app. Each account has its own data file.
// The password is never stored: only a salted hash of it (see Auth).
class User {
private:
    std::string username;       // used to log in, e.g. "khoa" (unique, case ignored)
    std::string nickname;       // shown in the app, e.g. "Hello, Khoa"
    std::string salt;           // random text mixed into the password hash
    std::string passwordHash;

public:
    User();
    User(const std::string& username, const std::string& nickname,
         const std::string& salt, const std::string& passwordHash);

    std::string getUsername() const { return username; }
    std::string getNickname() const { return nickname; }
    std::string getSalt() const { return salt; }
    std::string getPasswordHash() const { return passwordHash; }
};

#endif
