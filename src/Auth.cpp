#include "Auth.h"

#include <cstdint>
#include <fstream>
#include <random>

namespace Auth {

namespace {
    const int FILE_MAGIC = 0x55464D50;   // "PFMU": marks a users file

    std::string trim(const std::string& text) {
        int start = 0;
        int end = (int)text.size();
        while (start < end && (text[start] == ' ' || text[start] == '\t')) start++;
        while (end > start && (text[end - 1] == ' ' || text[end - 1] == '\t')) end--;
        return text.substr(start, end - start);
    }

    std::string toLower(const std::string& text) {
        std::string result = text;
        for (int i = 0; i < (int)result.size(); i++) {
            if (result[i] >= 'A' && result[i] <= 'Z') result[i] = (char)(result[i] - 'A' + 'a');
        }
        return result;
    }

    bool checkNewPassword(const std::string& password, const std::string& confirmPassword, std::string& message) {
        if ((int)password.size() < MIN_PASSWORD) {
            message = "The password must have at least " + std::to_string(MIN_PASSWORD) + " characters.";
            return false;
        }
        if (password != confirmPassword) {
            message = "The two passwords do not match.";
            return false;
        }
        return true;
    }

    void setNewPassword(User& user, const std::string& password) {
        std::string salt = makeSalt();
        user.setPassword(salt, hashPassword(password, salt));
    }

    bool isUsernameChar(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '.';
    }

    void writeString(std::ofstream& out, const std::string& s) {
        int len = (int)s.size();
        out.write(reinterpret_cast<const char*>(&len), sizeof(len));
        if (len > 0) out.write(s.data(), len);
    }

    bool readString(std::ifstream& in, std::string& s) {
        int len = 0;
        if (!in.read(reinterpret_cast<char*>(&len), sizeof(len))) return false;
        if (len < 0 || len > 1024) return false;
        if (len == 0) {
            s = "";
            return true;
        }
        char* buffer = new char[len];
        bool ok = (bool)in.read(buffer, len);
        if (ok) s.assign(buffer, len);
        delete[] buffer;
        return ok;
    }
}

bool loadUsers(DynamicArray<User>& users, const std::string& path) {
    users.clear();
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) return false;

    int magic = 0, count = 0;
    if (!in.read(reinterpret_cast<char*>(&magic), sizeof(magic)) || magic != FILE_MAGIC) return false;
    if (!in.read(reinterpret_cast<char*>(&count), sizeof(count)) || count < 0 || count > 100000) return false;

    DynamicArray<User> loaded;
    for (int i = 0; i < count; i++) {
        std::string username, nickname, salt, hash;
        if (!readString(in, username) || !readString(in, nickname) ||
            !readString(in, salt) || !readString(in, hash)) {
            return false;
        }
        loaded.add(User(username, nickname, salt, hash));
    }
    users = loaded;
    return true;
}

bool saveUsers(const DynamicArray<User>& users, const std::string& path) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) return false;
    int count = users.size();
    out.write(reinterpret_cast<const char*>(&FILE_MAGIC), sizeof(FILE_MAGIC));
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));
    for (int i = 0; i < count; i++) {
        writeString(out, users[i].getUsername());
        writeString(out, users[i].getNickname());
        writeString(out, users[i].getSalt());
        writeString(out, users[i].getPasswordHash());
    }
    return out.good();
}

int findUser(const DynamicArray<User>& users, const std::string& username) {
    std::string key = toLower(trim(username));
    for (int i = 0; i < users.size(); i++) {
        if (toLower(users[i].getUsername()) == key) return i;
    }
    return -1;
}

bool registerUser(DynamicArray<User>& users, const std::string& username, const std::string& nickname,
                  const std::string& password, const std::string& confirmPassword, std::string& message) {
    std::string name = trim(nickname);
    std::string user = trim(username);

    if (name.empty()) {
        message = "Please enter a nickname.";
        return false;
    }
    if ((int)name.size() > MAX_NICKNAME) {
        message = "The nickname can have at most " + std::to_string(MAX_NICKNAME) + " characters.";
        return false;
    }
    if ((int)user.size() < MIN_USERNAME || (int)user.size() > MAX_USERNAME) {
        message = "The username must have " + std::to_string(MIN_USERNAME) + " to " +
                  std::to_string(MAX_USERNAME) + " characters.";
        return false;
    }
    for (int i = 0; i < (int)user.size(); i++) {
        if (!isUsernameChar(user[i])) {
            message = "The username can only use letters, digits, '_' and '.'.";
            return false;
        }
    }
    if (findUser(users, user) != -1) {
        message = "The username \"" + user + "\" is already taken.";
        return false;
    }
    if (!checkNewPassword(password, confirmPassword, message)) return false;

    std::string salt = makeSalt();
    users.add(User(user, name, salt, hashPassword(password, salt)));
    message = "";
    return true;
}

int login(const DynamicArray<User>& users, const std::string& username, const std::string& password,
          std::string& message) {
    if (trim(username).empty() || password.empty()) {
        message = "Please enter your username and password.";
        return -1;
    }
    int index = findUser(users, username);
    if (index == -1 || hashPassword(password, users[index].getSalt()) != users[index].getPasswordHash()) {
        message = "Wrong username or password.";
        return -1;
    }
    message = "";
    return index;
}

bool resetPassword(DynamicArray<User>& users, const std::string& username, const std::string& nickname,
                   const std::string& newPassword, const std::string& confirmPassword, std::string& message) {
    if (trim(username).empty() || trim(nickname).empty()) {
        message = "Please enter your username and nickname.";
        return false;
    }
    int index = findUser(users, username);
    // Same message whether the username exists or not.
    if (index == -1 || toLower(trim(nickname)) != toLower(users[index].getNickname())) {
        message = "Username and nickname do not match.";
        return false;
    }
    if (!checkNewPassword(newPassword, confirmPassword, message)) return false;

    setNewPassword(users[index], newPassword);
    message = "";
    return true;
}

bool changePassword(DynamicArray<User>& users, int index, const std::string& currentPassword,
                    const std::string& newPassword, const std::string& confirmPassword, std::string& message) {
    if (index < 0 || index >= users.size()) {
        message = "Account not found.";
        return false;
    }
    User& user = users[index];
    if (hashPassword(currentPassword, user.getSalt()) != user.getPasswordHash()) {
        message = "The current password is wrong.";
        return false;
    }
    if (!checkNewPassword(newPassword, confirmPassword, message)) return false;
    if (newPassword == currentPassword) {
        message = "The new password must be different from the current one.";
        return false;
    }

    setNewPassword(user, newPassword);
    message = "";
    return true;
}

std::string hashPassword(const std::string& password, const std::string& salt) {
    const uint64_t OFFSET = 1469598103934665603ULL;
    const uint64_t PRIME = 1099511628211ULL;
    std::string text = salt + ":" + password;
    uint64_t hash = OFFSET;
    for (int round = 0; round < 1000; round++) {
        for (int i = 0; i < (int)text.size(); i++) {
            hash ^= (unsigned char)text[i];
            hash *= PRIME;
        }
        hash ^= (uint64_t)round;
        hash *= PRIME;
    }
    const char* digits = "0123456789abcdef";
    std::string result(16, '0');
    for (int i = 15; i >= 0; i--) {
        result[i] = digits[hash & 0xF];
        hash >>= 4;
    }
    return result;
}

std::string makeSalt() {
    static std::mt19937_64 generator(std::random_device{}());
    uint64_t value = generator();
    const char* digits = "0123456789abcdef";
    std::string result(16, '0');
    for (int i = 15; i >= 0; i--) {
        result[i] = digits[value & 0xF];
        value >>= 4;
    }
    return result;
}

std::string dataFileFor(const std::string& folder, const std::string& username) {
    return folder + "/" + toLower(trim(username)) + ".dat";
}

}
