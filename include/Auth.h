#ifndef AUTH_H
#define AUTH_H

#include <string>
#include "DynamicArray.h"
#include "User.h"

// Accounts: register, log in, and the file where each account keeps its data.
// Logic only (no GUI), so it can be unit tested.
namespace Auth {
    // Normal mode
    const std::string USERS_FILE = "data/users.dat";
    const std::string USERS_DIR = "data/users";
    // Demo mode (run_demo.bat) keeps its accounts apart from the real ones.
    const std::string DEMO_USERS_FILE = "data/demo_users.dat";
    const std::string DEMO_USERS_DIR = "data/demo_users";
    // Sample account of demo mode; its data is the sample data in data/demo.dat.
    const std::string DEMO_USERNAME = "demo";
    const std::string DEMO_PASSWORD = "demo123";
    const std::string DEMO_NICKNAME = "Demo";
    const std::string DEMO_DATA_FILE = "data/demo.dat";

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

    // Salted hash of a password (64-bit FNV-1a, repeated). Good enough for a class
    // project so passwords are not readable in the file; not a real security hash.
    std::string hashPassword(const std::string& password, const std::string& salt);
    std::string makeSalt();

    // Data file of an account: <folder>/<username in lower case>.dat
    std::string dataFileFor(const std::string& folder, const std::string& username);
}

#endif
