#ifndef STORAGE_H
#define STORAGE_H

#include <string>
#include "FinanceData.h"

// Binary file persistence (uses <fstream> only).
// Note: std::string must be written as [length][characters], never with
// sizeof(std::string), because a string only holds a pointer to its text.
namespace Storage {
    const std::string DEFAULT_FILE = "data/finance.dat";

    // Returns false if the file does not exist yet (first run) or is corrupted.
    bool load(FinanceData& data, const std::string& path = DEFAULT_FILE);
    bool save(const FinanceData& data, const std::string& path = DEFAULT_FILE);
}

#endif
