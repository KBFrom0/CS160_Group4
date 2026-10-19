# Personal Finance Manager

Final project – Introduction to Programming (CS160), FIT – VNU-HCMUS.
A console (CLI) application in C++ for managing personal income, expenses, wallets,
recurring transactions and statistics. Data is saved to a binary file.

## Team (Group 4)

| No. | Student ID | Name |
|-----|-----------|------|
| 2  | 23125084 | Nguyễn Đăng Khoa |
| 3  | 24125028 | Trương Thị Mỹ Duyên |
| 28 | 26125042 | Huỳnh Phước Trung Kiên |
| 36 | 26125060 | Dương Minh Quân |

## Build & Run

Requirement: `g++` with C++17 (MinGW-w64 / MSYS2 on Windows).

**Windows (quick):**
```bat
build.bat          :: build the app -> bin\PersonalFinanceManager.exe
build.bat test     :: build + run unit tests
bin\PersonalFinanceManager.exe
```

**Any OS (manual):**
```bash
g++ -std=c++17 -Wall -Wextra -Iinclude src/*.cpp -o PersonalFinanceManager
```

**CMake:**
```bash
cmake -S . -B build
cmake --build build
```

> Run the program from the project root folder so it finds `data/finance.dat`.

## Project Structure

```
CS160_Project/
├── include/                 # Header files
│   ├── DynamicArray.h       # Custom dynamic array (no STL containers allowed)
│   ├── Date.h               # Date: parse/validate/compare dd/mm/yyyy
│   ├── InputHelper.h        # Safe console input + formatting helpers
│   ├── ExpenseCategory.h    # Master data: id + name
│   ├── IncomeSource.h
│   ├── Wallet.h
│   ├── Income.h             # Transactions: store IDs, display names
│   ├── Expense.h
│   ├── RecurringTransaction.h
│   ├── FinanceData.h        # All lists of the app in one struct
│   ├── MasterData.h         # Lookup + CRUD for categories/sources/wallets (Duyen)
│   ├── Balance.h            # Wallet & total balance (Duyen)
│   ├── Transactions.h       # Add/list/edit/delete income & expense (Kien)
│   ├── Storage.h            # Binary save/load (Quan)
│   ├── Statistics.h         # Date-range stats + Statistics menu (Quan)
│   ├── Recurring.h          # Monthly recurring transactions (Khoa)
│   ├── AnnualStatistics.h   # Year-based stats (Khoa)
│   └── App.h                # Main loop, Dashboard, menu
├── src/                     # Implementations (.cpp) + main.cpp
├── tests/                   # Unit tests (test_core.cpp)
├── data/                    # Runtime data file (not committed)
├── docs/                    # Report, UML, work division
└── assets/
```

## Project Rules (from the assignment)

- **No STL containers** (`std::vector`, `std::list`, `std::map`, ...). Use `DynamicArray<T>`.
- `std::string` is allowed.
- Every entity is a `class`/`struct`.
- Transactions store **IDs** (wallet, category, source) and display **names**.
- After each function finishes, the app returns to the **Dashboard**.
- Persistence uses `<fstream>` with a **binary** file.

## Demo Videos

_(YouTube links will be added here.)_
