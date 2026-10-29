# Personal Finance Manager

Final project – Introduction to Programming (CS160), FIT – VNU-HCMUS.
A Windows desktop application in C++ for managing personal income, expenses, wallets,
recurring transactions and statistics, with a dashboard and charts.
Data is saved to a binary file.

## Team (Group 4)

| No. | Student ID | Name |
|-----|-----------|------|
| 2  | 23125084 | Nguyễn Đăng Khoa |
| 3  | 24125028 | Trương Thị Mỹ Duyên |
| 28 | 26125042 | Huỳnh Phước Trung Kiên |
| 36 | 26125060 | Dương Minh Quân |

## Build & Run

Install the requirements first: see **[REQUIREMENTS.md](REQUIREMENTS.md)** (only MSYS2 g++).

```bat
run_demo.bat       :: build + open the app with sample data   (double-click)
run.bat            :: build + open the app with your own data (double-click)
build.bat          :: only build -> bin\PersonalFinanceManager.exe
build.bat test     :: build + run unit tests
build.bat clean    :: delete bin\ (rebuilds the libraries next time)
```

The first build compiles the GUI libraries once (~2 minutes); later builds take a few seconds.

- Normal mode stores data in `data/finance.dat`.
- Demo mode (`--demo`) uses `data/demo.dat` with sample wallets, categories and transactions,
  so your real data is never touched.

**CMake (optional):**
```bash
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
```

## Screens

| Page | What it does |
|------|--------------|
| **Dashboard** | Total balance, this month's income / expense / savings, wallet balances (list + chart), recent transactions, last 6 months chart, recurring items |
| **Transactions** | Search and filter, add / edit / delete income and expense |
| **Recurring** | Monthly recurring income/expense with start date and optional end date |
| **Master Data** | Expense categories, income sources and wallets (add / rename / delete) |
| **Statistics** | By date range, by date range & wallet, annual overview, annual income by source, annual expense by category |
| **Settings** | Light / dark theme, data file info, team |

## Project Structure

```
CS160_Project/
├── include/
│   ├── DynamicArray.h        # Custom dynamic array (no STL containers allowed)
│   ├── Date.h                # Date: parse/validate/compare dd/mm/yyyy
│   ├── Format.h              # Money formatting
│   ├── ExpenseCategory.h, IncomeSource.h, Wallet.h       # Master data models
│   ├── Income.h, Expense.h, RecurringTransaction.h       # Transaction models
│   ├── FinanceData.h         # All lists of the app in one struct
│   ├── MasterData.h          # Logic: categories/sources/wallets  (Duyen)
│   ├── Balance.h             # Logic: wallet & total balance      (Duyen)
│   ├── Transactions.h        # Logic: income & expense            (Kien)
│   ├── Storage.h             # Logic: binary save/load            (Quan)
│   ├── Statistics.h          # Logic: date-range statistics       (Quan)
│   ├── Recurring.h           # Logic: monthly recurring           (Khoa)
│   ├── AnnualStatistics.h    # Logic: year-based statistics       (Khoa)
│   ├── DemoData.h            # Sample data for --demo
│   └── gui/                  # User interface (done)
│       ├── Gui.h             # App state, top bar, navigation
│       ├── Pages.h           # One function per page
│       ├── Theme.h           # Colors, fonts, light/dark
│       └── Widgets.h         # Cards, buttons, date picker, dialogs, toasts
├── src/                      # .cpp files (+ src/gui/ for the interface)
├── tests/                    # Unit tests (test_core.cpp)
├── third_party/              # Dear ImGui, ImPlot, icon header (see REQUIREMENTS.md)
├── assets/fonts/             # Font Awesome icons
├── data/                     # Runtime data files (not committed)
└── docs/                     # Report, UML, work division
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
