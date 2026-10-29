// Unit tests for the core building blocks (DynamicArray, Date, FinanceData::makeId).
// Build & run: see README.md. Prints FAIL lines and returns 1 if anything breaks.
#include <iostream>
#include <string>
#include "DynamicArray.h"
#include "Date.h"
#include "FinanceData.h"
#include "Format.h"

static int failures = 0;

#define CHECK(cond)                                                        \
    do {                                                                   \
        if (!(cond)) {                                                     \
            std::cout << "FAIL " << __FILE__ << ":" << __LINE__ << "  "   \
                      << #cond << '\n';                                    \
            failures++;                                                    \
        }                                                                  \
    } while (0)

static void testDynamicArray() {
    DynamicArray<int> a;
    CHECK(a.size() == 0);
    CHECK(a.isEmpty());

    for (int i = 0; i < 100; i++) a.add(i);
    CHECK(a.size() == 100);
    CHECK(a[0] == 0 && a[99] == 99);

    a.removeAt(0);
    CHECK(a.size() == 99);
    CHECK(a[0] == 1);
    a.removeAt(a.size() - 1);
    CHECK(a[a.size() - 1] == 98);

    // Copies must be independent (deep copy).
    DynamicArray<int> b = a;
    b.set(0, -1);
    CHECK(a[0] == 1);
    CHECK(b[0] == -1);

    DynamicArray<int> c;
    c = a;
    c.add(500);
    CHECK(c.size() == a.size() + 1);

    c = c;  // self-assignment must be safe
    CHECK(c.size() == a.size() + 1);

    bool threw = false;
    try { a.get(1000); } catch (const std::out_of_range&) { threw = true; }
    CHECK(threw);

    DynamicArray<std::string> s;
    s.add("Food");
    s.add("Rent");
    CHECK(s[1] == "Rent");
    s.clear();
    CHECK(s.isEmpty());
}

static void testDate() {
    Date d;
    CHECK(Date::parse("29/02/2024", d));      // leap year
    CHECK(!Date::parse("29/02/2023", d));     // not leap
    CHECK(!Date::parse("31/04/2025", d));
    CHECK(!Date::parse("1-1-2025", d));
    CHECK(!Date::parse("01/01/2025x", d));
    CHECK(Date::parse("5/3/2025", d));
    CHECK(d.toString() == "05/03/2025");

    Date a(1, 1, 2025), b(31, 12, 2025), mid(15, 6, 2025);
    CHECK(a < b);
    CHECK(mid.isBetween(a, b));
    CHECK(a.isBetween(a, b));
    CHECK(!Date(1, 1, 2026).isBetween(a, b));
    CHECK(Date::today().isValid());
}

static void testHelpers() {
    CHECK(FinanceData::makeId("CAT", 7, 3) == "CAT007");
    CHECK(FinanceData::makeId("INC", 12345, 4) == "INC12345");
    CHECK(Format::money(1500000) == "1,500,000");
    CHECK(Format::money(-999) == "-999");
    CHECK(Format::money(0) == "0");
}

int main() {
    testDynamicArray();
    testDate();
    testHelpers();
    if (failures == 0) {
        std::cout << "All tests passed.\n";
        return 0;
    }
    std::cout << failures << " test(s) failed.\n";
    return 1;
}
