#include "ExpenseCategory.h"

ExpenseCategory::ExpenseCategory() : id(""), name("") {}

ExpenseCategory::ExpenseCategory(const std::string& id, const std::string& name)
    : id(id), name(name) {}
