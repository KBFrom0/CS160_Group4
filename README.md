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
run.bat            :: build + open the app (double-click)
build.bat          :: only build -> bin\PersonalFinanceManager.exe
build.bat test     :: build + run unit tests
build.bat clean    :: delete bin\ (rebuilds the libraries next time)
```

The first build compiles the GUI libraries once (~2 minutes); later builds take a few seconds.

### First run

1. Run `run.bat`. The app opens on the **Get Started** screen.
2. Click **Create account** and enter a nickname, a username (3–20 letters, digits, `_` or `.`)
   and a password (at least 6 characters). You are logged in right away.
3. Add your wallets, expense categories and income sources in **Master Data**, then add
   transactions from the **Dashboard** or **Transactions**.
4. Use the **Log out** button (top-right) to switch account. Next time, click **Log in**.

### Data files

| File | Content |
|------|---------|
| `data/users.dat` | All accounts (username, nickname, salted password hash) |
| `data/users/<username>.dat` | Wallets, categories, sources, transactions and recurring items of one account |
| `data/settings.cfg` | Light / dark theme |

All data is saved automatically after every change and when the app closes.
To start over with no accounts, close the app and delete `data\users.dat` and the `data\users\` folder.

**CMake (optional):**
```bash
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
```

## Screens

| Page | What it does |
|------|--------------|
| **Get Started** | Team photos around the app logo; **Log in** / **Create account** (nickname, username, password) |
| **Dashboard** | "Hello, *nickname*!", total balance, this month's income / expense / savings, wallet balances (list + chart), recent transactions, last 6 months chart, recurring items |
| **Transactions** | Search and filter, add / edit / delete income and expense |
| **Recurring** | Monthly recurring income/expense with start date and optional end date |
| **Master Data** | Expense categories, income sources and wallets (add / rename / delete) |
| **Statistics** | By date range, by date range & wallet, annual overview, annual income by source, annual expense by category |
| **Settings** | Light / dark theme, account and data file info, team |

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
│   ├── User.h                # Account model (username, nickname, password hash)
│   ├── Auth.h                # Logic: register / log in, accounts file (Khoa)
│   └── gui/                  # User interface
│       ├── Gui.h             # App state, log in / log out, top bar, navigation
│       ├── Pages.h           # One function per page (+ Get Started / Log in / Create account)
│       ├── Images.h          # Loads PNG images (team photos) with Windows Imaging Component
│       ├── Theme.h           # Colors, fonts, light/dark
│       └── Widgets.h         # Cards, buttons, date picker, dialogs, toasts
├── src/                      # .cpp files (+ src/gui/ for the interface)
├── tests/                    # Unit tests (test_core.cpp)
├── third_party/              # Dear ImGui, ImPlot, icon header (see REQUIREMENTS.md)
├── assets/fonts/             # Font Awesome icons
├── assets/images/            # Team photos for the Get Started screen
├── data/                     # Runtime data files (not committed)
└── docs/                     # Weekly reports (final report, UML, work division: to be added)
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
