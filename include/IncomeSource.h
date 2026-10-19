#ifndef INCOME_SOURCE_H
#define INCOME_SOURCE_H

#include <string>

// Money source, e.g. Salary, Bonus, Savings Interest.
class IncomeSource {
private:
    std::string id;    // e.g. "SRC001"
    std::string name;  // e.g. "Salary"

public:
    IncomeSource();
    IncomeSource(const std::string& id, const std::string& name);

    std::string getId() const { return id; }
    std::string getName() const { return name; }
    void setName(const std::string& newName) { name = newName; }
};

#endif
