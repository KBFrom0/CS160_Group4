#ifndef INPUT_HELPER_H
#define INPUT_HELPER_H

#include <string>
#include "Date.h"

// Shared console input/output helpers. Every input function re-prompts until
// the user enters a valid value, so callers never need to validate again.
namespace InputHelper {
    int inputInt(const std::string& prompt, int min, int max);
    long long inputAmount(const std::string& prompt);  // must be > 0
    std::string inputString(const std::string& prompt, bool allowEmpty = false);
    Date inputDate(const std::string& prompt);
    // Empty input means "no date"; returns false in that case.
    bool inputOptionalDate(const std::string& prompt, Date& result);
    bool confirm(const std::string& prompt);  // y/n

    void clearScreen();
    void pause();
    void showNotImplemented(const std::string& feature);  // placeholder for TODO features
    void printLine(char ch = '-', int width = 60);
    std::string formatMoney(long long amount);  // 1500000 -> "1,500,000"
}

#endif
