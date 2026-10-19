#ifndef EXPENSE_CATEGORY_H
#define EXPENSE_CATEGORY_H

#include <string>

// Expense type, e.g. Food, Rent, Transportation.
class ExpenseCategory {
private:
    std::string id;    // e.g. "CAT001"
    std::string name;  // e.g. "Food"

public:
    ExpenseCategory();
    ExpenseCategory(const std::string& id, const std::string& name);

    std::string getId() const { return id; }
    std::string getName() const { return name; }
    void setName(const std::string& newName) { name = newName; }
};

#endif
