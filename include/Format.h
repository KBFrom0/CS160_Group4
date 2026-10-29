#ifndef FORMAT_H
#define FORMAT_H

#include <string>

// Text formatting helpers shared by the UI.
namespace Format {
    std::string money(long long amount);         // 1500000 -> "1,500,000"
    std::string signedMoney(long long amount);   // 1500000 -> "+1,500,000", -5 -> "-5"
    std::string monthName(int month);            // 1 -> "Jan"
}

#endif
