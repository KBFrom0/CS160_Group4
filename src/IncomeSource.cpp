#include "IncomeSource.h"

IncomeSource::IncomeSource() : id(""), name("") {}

IncomeSource::IncomeSource(const std::string& id, const std::string& name)
    : id(id), name(name) {}
