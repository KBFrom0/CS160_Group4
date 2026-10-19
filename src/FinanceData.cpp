#include "FinanceData.h"

std::string FinanceData::makeId(const std::string& prefix, int number, int width) {
    std::string digits = std::to_string(number);
    while ((int)digits.size() < width) {
        digits = "0" + digits;
    }
    return prefix + digits;
}
