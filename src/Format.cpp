#include "Format.h"

namespace Format {

std::string money(long long amount) {
    bool negative = amount < 0;
    unsigned long long value = negative ? -(unsigned long long)amount : amount;
    std::string digits = std::to_string(value);
    std::string result;
    int counter = 0;
    for (int i = (int)digits.size() - 1; i >= 0; i--) {
        result = digits[i] + result;
        if (++counter % 3 == 0 && i > 0) {
            result = ',' + result;
        }
    }
    return negative ? "-" + result : result;
}

std::string signedMoney(long long amount) {
    return amount > 0 ? "+" + money(amount) : money(amount);
}

std::string monthName(int month) {
    const char* names[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    return (month >= 1 && month <= 12) ? names[month - 1] : "?";
}

}
