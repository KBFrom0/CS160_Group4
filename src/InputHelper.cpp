#include "InputHelper.h"

#include <iostream>
#include <sstream>
#include <cstdlib>

namespace {
    std::string readLine() {
        std::string line;
        if (!std::getline(std::cin, line)) {
            // stdin closed (e.g. Ctrl+Z): exit instead of looping forever.
            std::cout << "\nInput closed. Exiting.\n";
            std::exit(0);
        }
        return line;
    }

    std::string trim(const std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        size_t end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }
}

namespace InputHelper {

int inputInt(const std::string& prompt, int min, int max) {
    while (true) {
        std::cout << prompt;
        std::istringstream in(trim(readLine()));
        int value;
        std::string rest;
        if (in >> value && !(in >> rest) && value >= min && value <= max) {
            return value;
        }
        std::cout << "  Invalid input. Please enter a number from "
                  << min << " to " << max << ".\n";
    }
}

long long inputAmount(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        std::istringstream in(trim(readLine()));
        long long value;
        std::string rest;
        if (in >> value && !(in >> rest) && value > 0) {
            return value;
        }
        std::cout << "  Invalid amount. Please enter a positive whole number.\n";
    }
}

std::string inputString(const std::string& prompt, bool allowEmpty) {
    while (true) {
        std::cout << prompt;
        std::string value = trim(readLine());
        if (!value.empty() || allowEmpty) {
            return value;
        }
        std::cout << "  This field cannot be empty.\n";
    }
}

Date inputDate(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        Date result;
        if (Date::parse(trim(readLine()), result)) {
            return result;
        }
        std::cout << "  Invalid date. Use format dd/mm/yyyy.\n";
    }
}

bool inputOptionalDate(const std::string& prompt, Date& result) {
    while (true) {
        std::cout << prompt;
        std::string text = trim(readLine());
        if (text.empty()) return false;
        if (Date::parse(text, result)) return true;
        std::cout << "  Invalid date. Use format dd/mm/yyyy, or leave blank.\n";
    }
}

bool confirm(const std::string& prompt) {
    while (true) {
        std::cout << prompt << " (y/n): ";
        std::string answer = trim(readLine());
        if (answer == "y" || answer == "Y") return true;
        if (answer == "n" || answer == "N") return false;
    }
}

void clearScreen() {
#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
}

void pause() {
    std::cout << "\nPress Enter to continue...";
    readLine();
}

void printLine(char ch, int width) {
    std::cout << std::string(width, ch) << '\n';
}

std::string formatMoney(long long amount) {
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

}
