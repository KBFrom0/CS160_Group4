#include "Storage.h"

#include <fstream>

namespace Storage {

bool load(FinanceData& data, const std::string& path) {
    // TODO: read every list and the nextXxxNo counters from the binary file.
    (void)data;
    (void)path;
    return false;
}

bool save(const FinanceData& data, const std::string& path) {
    // TODO: write every list and the nextXxxNo counters to the binary file.
    (void)data;
    (void)path;
    return true;
}

}
