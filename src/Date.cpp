#include "Date.h"

#include <ctime>
#include <sstream>
#include <iomanip>

Date::Date() : day(1), month(1), year(2000) {}

Date::Date(int day, int month, int year) : day(day), month(month), year(year) {}

bool Date::isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int Date::daysInMonth(int month, int year) {
    switch (month) {
        case 2:
            return isLeapYear(year) ? 29 : 28;
        case 4: case 6: case 9: case 11:
            return 30;
        default:
            return 31;
    }
}

bool Date::isValid() const {
    if (year < 1900 || year > 9999) return false;
    if (month < 1 || month > 12) return false;
    return day >= 1 && day <= daysInMonth(month, year);
}

std::string Date::toString() const {
    std::ostringstream out;
    out << std::setfill('0') << std::setw(2) << day << '/'
        << std::setw(2) << month << '/'
        << std::setw(4) << year;
    return out.str();
}

bool Date::parse(const std::string& text, Date& result) {
    std::istringstream in(text);
    int d, m, y;
    char sep1, sep2;
    if (!(in >> d >> sep1 >> m >> sep2 >> y)) return false;
    if (sep1 != '/' || sep2 != '/') return false;

    std::string rest;
    if (in >> rest) return false;  // trailing garbage

    Date candidate(d, m, y);
    if (!candidate.isValid()) return false;
    result = candidate;
    return true;
}

Date Date::today() {
    std::time_t now = std::time(nullptr);
    std::tm* local = std::localtime(&now);
    return Date(local->tm_mday, local->tm_mon + 1, local->tm_year + 1900);
}

int Date::compare(const Date& a, const Date& b) {
    if (a.year != b.year) return a.year - b.year;
    if (a.month != b.month) return a.month - b.month;
    return a.day - b.day;
}

bool Date::isBetween(const Date& from, const Date& to) const {
    return *this >= from && *this <= to;
}
