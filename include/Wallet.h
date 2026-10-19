#ifndef WALLET_H
#define WALLET_H

#include <string>

// Money storage location, e.g. Cash, Bank Account, Credit Card.
// The balance is NOT stored here: it is computed from transactions so it can
// never get out of sync with them.
class Wallet {
private:
    std::string id;    // e.g. "WAL001"
    std::string name;  // e.g. "Cash"

public:
    Wallet();
    Wallet(const std::string& id, const std::string& name);

    std::string getId() const { return id; }
    std::string getName() const { return name; }
    void setName(const std::string& newName) { name = newName; }
};

#endif
