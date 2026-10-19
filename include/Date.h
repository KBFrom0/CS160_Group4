#ifndef DATE_H
#define DATE_H

#include <string>

class Date {
private:
    int day;
    int month;
    int year;

public:
    Date();
    Date(int day, int month, int year);

    int getDay() const { return day; }
    int getMonth() const { return month; }
    int getYear() const { return year; }

    bool isValid() const;
    std::string toString() const;  // "dd/mm/yyyy"

    // Parses "dd/mm/yyyy". Returns false (and leaves result unchanged) if invalid.
    static bool parse(const std::string& text, Date& result);
    static Date today();
    static bool isLeapYear(int year);
    static int daysInMonth(int month, int year);

    // Negative if a < b, 0 if equal, positive if a > b.
    static int compare(const Date& a, const Date& b);

    bool operator<(const Date& other) const { return compare(*this, other) < 0; }
    bool operator>(const Date& other) const { return compare(*this, other) > 0; }
    bool operator<=(const Date& other) const { return compare(*this, other) <= 0; }
    bool operator>=(const Date& other) const { return compare(*this, other) >= 0; }
    bool operator==(const Date& other) const { return compare(*this, other) == 0; }
    bool operator!=(const Date& other) const { return compare(*this, other) != 0; }

    // True if this date lies within [from, to] (inclusive).
    bool isBetween(const Date& from, const Date& to) const;
};

#endif
