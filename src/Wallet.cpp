#include "Wallet.h"

Wallet::Wallet() : id(""), name("") {}

Wallet::Wallet(const std::string& id, const std::string& name)
    : id(id), name(name) {}
