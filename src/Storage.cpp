#include "Storage.h"

#include <fstream>

namespace Storage {

static bool writeString(std::ofstream &out, const std::string &s) {
  int len = static_cast<int>(s.size());
  out.write(reinterpret_cast<const char *>(&len), sizeof(len));
  if (len > 0) {
    out.write(s.data(), len);
  }
  return out.good();
}

static bool readString(std::ifstream &in, std::string &s) {
  int len = 0;
  if (!in.read(reinterpret_cast<char *>(&len), sizeof(len)))
    return false;
  if (len < 0 || len > 65535)
    return false;
  if (len == 0) {
    s = "";
    return true;
  }
  char *buf = new char[len];
  if (!in.read(buf, len)) {
    delete[] buf;
    return false;
  }
  s.assign(buf, len);
  delete[] buf;
  return true;
}

static bool writeDate(std::ofstream &out, const Date &d) {
  int day = d.getDay();
  int month = d.getMonth();
  int year = d.getYear();
  out.write(reinterpret_cast<const char *>(&day), sizeof(day));
  out.write(reinterpret_cast<const char *>(&month), sizeof(month));
  out.write(reinterpret_cast<const char *>(&year), sizeof(year));
  return out.good();
}

static bool readDate(std::ifstream &in, Date &d) {
  int day = 0, month = 0, year = 0;
  if (!in.read(reinterpret_cast<char *>(&day), sizeof(day)))
    return false;
  if (!in.read(reinterpret_cast<char *>(&month), sizeof(month)))
    return false;
  if (!in.read(reinterpret_cast<char *>(&year), sizeof(year)))
    return false;
  d = Date(day, month, year);
  return true;
}

bool save(const FinanceData &data, const std::string &path) {
  std::ofstream out(path, std::ios::binary);
  if (!out.is_open())
    return false;

  out.write(reinterpret_cast<const char *>(&data.nextCategoryNo),
            sizeof(data.nextCategoryNo));
  out.write(reinterpret_cast<const char *>(&data.nextSourceNo),
            sizeof(data.nextSourceNo));
  out.write(reinterpret_cast<const char *>(&data.nextWalletNo),
            sizeof(data.nextWalletNo));
  out.write(reinterpret_cast<const char *>(&data.nextIncomeNo),
            sizeof(data.nextIncomeNo));
  out.write(reinterpret_cast<const char *>(&data.nextExpenseNo),
            sizeof(data.nextExpenseNo));
  out.write(reinterpret_cast<const char *>(&data.nextRecurringNo),
            sizeof(data.nextRecurringNo));

  int count = data.categories.size();
  out.write(reinterpret_cast<const char *>(&count), sizeof(count));
  for (int i = 0; i < count; i++) {
    const ExpenseCategory &c = data.categories.get(i);
    writeString(out, c.getId());
    writeString(out, c.getName());
  }

  count = data.sources.size();
  out.write(reinterpret_cast<const char *>(&count), sizeof(count));
  for (int i = 0; i < count; i++) {
    const IncomeSource &s = data.sources.get(i);
    writeString(out, s.getId());
    writeString(out, s.getName());
  }

  count = data.wallets.size();
  out.write(reinterpret_cast<const char *>(&count), sizeof(count));
  for (int i = 0; i < count; i++) {
    const Wallet &w = data.wallets.get(i);
    writeString(out, w.getId());
    writeString(out, w.getName());
  }

  count = data.incomes.size();
  out.write(reinterpret_cast<const char *>(&count), sizeof(count));
  for (int i = 0; i < count; i++) {
    const Income &inc = data.incomes.get(i);
    writeString(out, inc.getId());
    writeDate(out, inc.getDate());
    writeString(out, inc.getSourceId());
    long long amount = inc.getAmount();
    out.write(reinterpret_cast<const char *>(&amount), sizeof(amount));
    writeString(out, inc.getWalletId());
    writeString(out, inc.getDescription());
    writeString(out, inc.getRecurringId());
  }

  count = data.expenses.size();
  out.write(reinterpret_cast<const char *>(&count), sizeof(count));
  for (int i = 0; i < count; i++) {
    const Expense &exp = data.expenses.get(i);
    writeString(out, exp.getId());
    writeDate(out, exp.getDate());
    writeString(out, exp.getCategoryId());
    long long amount = exp.getAmount();
    out.write(reinterpret_cast<const char *>(&amount), sizeof(amount));
    writeString(out, exp.getWalletId());
    writeString(out, exp.getDescription());
    writeString(out, exp.getRecurringId());
  }

  count = data.recurrings.size();
  out.write(reinterpret_cast<const char *>(&count), sizeof(count));
  for (int i = 0; i < count; i++) {
    const RecurringTransaction &r = data.recurrings.get(i);
    writeString(out, r.getId());
    int type = static_cast<int>(r.getType());
    out.write(reinterpret_cast<const char *>(&type), sizeof(type));
    writeString(out, r.getRefId());
    long long amount = r.getAmount();
    out.write(reinterpret_cast<const char *>(&amount), sizeof(amount));
    writeString(out, r.getWalletId());
    writeString(out, r.getDescription());
    writeDate(out, r.getStartDate());
    bool hasEnd = r.getHasEndDate();
    out.write(reinterpret_cast<const char *>(&hasEnd), sizeof(hasEnd));
    writeDate(out, r.getEndDate());
  }

  return out.good();
}

bool load(FinanceData &data, const std::string &path) {
  std::ifstream in(path, std::ios::binary);
  if (!in.is_open())
    return false;

  FinanceData tmp;

  if (!in.read(reinterpret_cast<char *>(&tmp.nextCategoryNo),
               sizeof(tmp.nextCategoryNo)))
    return false;
  if (!in.read(reinterpret_cast<char *>(&tmp.nextSourceNo),
               sizeof(tmp.nextSourceNo)))
    return false;
  if (!in.read(reinterpret_cast<char *>(&tmp.nextWalletNo),
               sizeof(tmp.nextWalletNo)))
    return false;
  if (!in.read(reinterpret_cast<char *>(&tmp.nextIncomeNo),
               sizeof(tmp.nextIncomeNo)))
    return false;
  if (!in.read(reinterpret_cast<char *>(&tmp.nextExpenseNo),
               sizeof(tmp.nextExpenseNo)))
    return false;
  if (!in.read(reinterpret_cast<char *>(&tmp.nextRecurringNo),
               sizeof(tmp.nextRecurringNo)))
    return false;

  if (tmp.nextCategoryNo < 1 || tmp.nextSourceNo < 1 || tmp.nextWalletNo < 1 ||
      tmp.nextIncomeNo < 1 || tmp.nextExpenseNo < 1 || tmp.nextRecurringNo < 1)
    return false;

  int count = 0;

  if (!in.read(reinterpret_cast<char *>(&count), sizeof(count)))
    return false;
  if (count < 0 || count > 100000)
    return false;
  for (int i = 0; i < count; i++) {
    std::string id, name;
    if (!readString(in, id))
      return false;
    if (!readString(in, name))
      return false;
    tmp.categories.add(ExpenseCategory(id, name));
  }

  if (!in.read(reinterpret_cast<char *>(&count), sizeof(count)))
    return false;
  if (count < 0 || count > 100000)
    return false;
  for (int i = 0; i < count; i++) {
    std::string id, name;
    if (!readString(in, id))
      return false;
    if (!readString(in, name))
      return false;
    tmp.sources.add(IncomeSource(id, name));
  }

  if (!in.read(reinterpret_cast<char *>(&count), sizeof(count)))
    return false;
  if (count < 0 || count > 100000)
    return false;
  for (int i = 0; i < count; i++) {
    std::string id, name;
    if (!readString(in, id))
      return false;
    if (!readString(in, name))
      return false;
    tmp.wallets.add(Wallet(id, name));
  }

  if (!in.read(reinterpret_cast<char *>(&count), sizeof(count)))
    return false;
  if (count < 0 || count > 100000)
    return false;
  for (int i = 0; i < count; i++) {
    std::string id, sourceId, walletId, description, recurringId;
    Date date;
    long long amount = 0;
    if (!readString(in, id))
      return false;
    if (!readDate(in, date))
      return false;
    if (!readString(in, sourceId))
      return false;
    if (!in.read(reinterpret_cast<char *>(&amount), sizeof(amount)))
      return false;
    if (!readString(in, walletId))
      return false;
    if (!readString(in, description))
      return false;
    if (!readString(in, recurringId))
      return false;
    tmp.incomes.add(
        Income(id, date, sourceId, amount, walletId, description, recurringId));
  }

  if (!in.read(reinterpret_cast<char *>(&count), sizeof(count)))
    return false;
  if (count < 0 || count > 100000)
    return false;
  for (int i = 0; i < count; i++) {
    std::string id, categoryId, walletId, description, recurringId;
    Date date;
    long long amount = 0;
    if (!readString(in, id))
      return false;
    if (!readDate(in, date))
      return false;
    if (!readString(in, categoryId))
      return false;
    if (!in.read(reinterpret_cast<char *>(&amount), sizeof(amount)))
      return false;
    if (!readString(in, walletId))
      return false;
    if (!readString(in, description))
      return false;
    if (!readString(in, recurringId))
      return false;
    tmp.expenses.add(Expense(id, date, categoryId, amount, walletId,
                             description, recurringId));
  }

  if (!in.read(reinterpret_cast<char *>(&count), sizeof(count)))
    return false;
  if (count < 0 || count > 100000)
    return false;
  for (int i = 0; i < count; i++) {
    std::string id, refId, walletId, description;
    int typeInt = 0;
    long long amount = 0;
    Date startDate, endDate;
    bool hasEnd = false;
    if (!readString(in, id))
      return false;
    if (!in.read(reinterpret_cast<char *>(&typeInt), sizeof(typeInt)))
      return false;
    if (typeInt != 0 && typeInt != 1)
      return false;
    if (!readString(in, refId))
      return false;
    if (!in.read(reinterpret_cast<char *>(&amount), sizeof(amount)))
      return false;
    if (!readString(in, walletId))
      return false;
    if (!readString(in, description))
      return false;
    if (!readDate(in, startDate))
      return false;
    if (!in.read(reinterpret_cast<char *>(&hasEnd), sizeof(hasEnd)))
      return false;
    if (!readDate(in, endDate))
      return false;
    TransactionType type = static_cast<TransactionType>(typeInt);
    tmp.recurrings.add(RecurringTransaction(id, type, refId, amount, walletId,
                                            description, startDate, hasEnd,
                                            endDate));
  }

  data = tmp;
  return true;
}

} // namespace Storage
