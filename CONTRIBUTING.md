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
- Ngày dùng class `Date`.
- Giao dịch chỉ lưu **ID**; khi hiển thị thì tra ra **Name**.
- Chữ hiển thị trên giao diện dùng tiếng Anh.

### Giao diện (GUI) đã làm sẵn
- Toàn bộ giao diện cửa sổ nằm trong `src/gui/` và **đã xong**: Dashboard, Transactions,
  Recurring, Master Data, Statistics, Settings, hộp thoại thêm/sửa/xóa, chọn ngày,
  thông báo, giao diện sáng/tối.
- Việc của mỗi người là điền code vào các hàm nằm dưới dòng
  `// ===== Logic (TODO: <tên>) =====` trong file `.cpp` của mình (`src/`, không phải `src/gui/`).
- Hàm logic **không vẽ gì, không hỏi người dùng** — chỉ nhận tham số, xử lý `FinanceData`
  rồi trả kết quả. Giao diện tự hiển thị kết quả, tự lưu file và tự quay về Dashboard.
- Báo lỗi cho người dùng bằng cách trả `false` / `""` và ghi lý do vào `message`
  (ví dụ `MasterData::addItem`), giao diện sẽ hiện thông báo đỏ.
- **Không sửa `src/gui/`** trừ khi đã báo nhóm.
- Test bằng `run_demo.bat` (dữ liệu mẫu). Chỗ nào đang chờ hàm của bạn sẽ hiện `0`,
  `(unknown)`, danh sách rỗng, hoặc dòng cam *"Waiting for ..."* / *"... is not implemented yet"*.
  Khi hàm chạy đúng, chỗ đó tự hiện số liệu.

| Màn hình đang chờ | Hàm cần viết | Người |
|---|---|---|
| Tên trong bảng, chặn xóa, Master Data (Add/Rename/Delete) | `MasterData::*` | Duyên |
| Số dư ví, tổng số dư (Dashboard) | `Balance::*` | Duyên |
| Thêm/sửa/xóa, danh sách giao dịch, "Recent transactions" | `Transactions::*` | Kiên |
| Dữ liệu còn sau khi tắt app | `Storage::save/load` | Quân |
| Thẻ "This month", biểu đồ 6 tháng, Statistics (Date range, By wallet) | `Statistics::summarize` | Quân |
| Tự thêm giao dịch hằng tháng, trang Recurring | `Recurring::*` | Khoa |
| Statistics (Annual overview, Income by source, Expense by category) | `AnnualStatistics::*` | Khoa |

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
