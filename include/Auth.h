#ifndef AUTH_H
#define AUTH_H

#include <string>
#include "DynamicArray.h"
#include "User.h"

// Accounts: register, log in, and the file where each account keeps its data.
// Logic only (no GUI), so it can be unit tested.
namespace Auth {
    const std::string USERS_FILE = "data/users.dat";
    const std::string USERS_DIR = "data/users";

    const int MIN_USERNAME = 3;
    const int MAX_USERNAME = 20;
    const int MIN_PASSWORD = 6;
    const int MAX_NICKNAME = 20;

    // Binary file with all accounts. load: false if the file is missing or damaged
    // (the list is then left empty).
    bool loadUsers(DynamicArray<User>& users, const std::string& path);
    bool saveUsers(const DynamicArray<User>& users, const std::string& path);

    // Index of the account (case ignored), or -1.
    int findUser(const DynamicArray<User>& users, const std::string& username);

    // Checks the form and adds the account. On failure returns false and
    // explains why in `message`.
    bool registerUser(DynamicArray<User>& users, const std::string& username, const std::string& nickname,
                      const std::string& password, const std::string& confirmPassword, std::string& message);

    // Index of the account if the username and password match, else -1 with a message.
    int login(const DynamicArray<User>& users, const std::string& username, const std::string& password,
              std::string& message);

    // Forgot password: the nickname must match the account (case ignored).
    // Sets the new password; on failure returns false with a message.
    bool resetPassword(DynamicArray<User>& users, const std::string& username, const std::string& nickname,
                       const std::string& newPassword, const std::string& confirmPassword, std::string& message);

    // Change password of users[index] after checking the current one.
    bool changePassword(DynamicArray<User>& users, int index, const std::string& currentPassword,
                        const std::string& newPassword, const std::string& confirmPassword, std::string& message);

    // Salted hash of a password (64-bit FNV-1a, repeated). Good enough for a class
    // project so passwords are not readable in the file; not a real security hash.
    std::string hashPassword(const std::string& password, const std::string& salt);
    std::string makeSalt();

    // Data file of an account: <folder>/<username in lower case>.dat
    std::string dataFileFor(const std::string& folder, const std::string& username);
}

#endif
