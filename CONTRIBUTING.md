# Quy ước làm việc nhóm

> **Bắt buộc:** mỗi thành viên commit bằng **tài khoản GitHub của chính mình**.
> Ai không có commit trong repo sẽ bị **0 điểm**. Không push code hộ người khác.

## 1. Lấy code về lần đầu

```bash
git clone <link-repo>
cd CS160_Project
build.bat test      # phải in "All tests passed."
```

Kiểm tra git đã đúng tên/email GitHub của mình chưa:
```bash
git config --global user.name  "Ten GitHub cua ban"
git config --global user.email "email-dang-ky-github@example.com"
```

## 2. Quy trình mỗi task

1. Cập nhật `main`: `git checkout main && git pull`
2. Tạo nhánh mới cho task: `git checkout -b feature/<ten-ngan>`
   ví dụ `feature/master-data`, `feature/add-income`, `fix/date-parse`
3. Code, **build và chạy test trước khi commit**: `build.bat test`
4. Commit nhỏ, thường xuyên:
   `git add <file>` → `git commit -m "Add wallet CRUD menu"`
5. Push: `git push -u origin feature/<ten-ngan>`
6. Tạo **Pull Request** vào `main`, nhờ 1 người khác review rồi mới merge.

Không push thẳng lên `main`.

## 3. Commit message

Tiếng Anh, bắt đầu bằng động từ, ngắn gọn:
- `Add expense category CRUD`
- `Fix wallet balance when deleting expense`
- `Update README build steps`

## 4. Quy ước code

| Thứ | Quy ước | Ví dụ |
|-----|---------|-------|
| Class | PascalCase | `ExpenseCategory` |
| Hàm, biến | camelCase | `getNameById`, `totalIncome` |
| Hằng số | UPPER_CASE | `DEFAULT_FILE` |
| File | trùng tên class | `Wallet.h`, `Wallet.cpp` |
| Thụt lề | 4 dấu cách | |

- Header luôn có include guard (`#ifndef ..._H`).
- **Cấm** `std::vector`, `std::list`, `std::map`, ... → dùng `DynamicArray<T>`.
- Tiền lưu bằng `long long` (VND, số nguyên) — không dùng `double` cho tiền.
- Ngày dùng class `Date`, nhập bằng `InputHelper::inputDate`.
- Mọi nhập liệu dùng hàm trong `InputHelper` (đã tự kiểm tra input sai).
- Giao dịch chỉ lưu **ID**; khi hiển thị thì tra ra **Name**.
- Xong một chức năng thì `return` về `App::run()` → tự quay về Dashboard.
- Chữ hiển thị trên console dùng tiếng Anh (tránh lỗi font tiếng Việt trên cmd).

## 5. Định dạng ID

| Loại | Prefix | Ví dụ | Bộ đếm trong `FinanceData` |
|------|--------|-------|----------------------------|
| Expense Category | `CAT` | `CAT001` | `nextCategoryNo` |
| Income Source | `SRC` | `SRC001` | `nextSourceNo` |
| Wallet | `WAL` | `WAL001` | `nextWalletNo` |
| Income | `INC` | `INC0001` | `nextIncomeNo` |
| Expense | `EXP` | `EXP0001` | `nextExpenseNo` |
| Recurring | `REC` | `REC001` | `nextRecurringNo` |

Tạo ID: `FinanceData::makeId("CAT", data.nextCategoryNo++, 3)`.
ID đã xóa **không** dùng lại.
