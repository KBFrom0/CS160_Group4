#ifndef DEMO_DATA_H
#define DEMO_DATA_H

#include "FinanceData.h"

// Sample data for testing the UI: run the app with --demo.
namespace DemoData {
    // Replaces everything in `data` with sample wallets, categories, sources,
    // transactions (2025-2026) and recurring items.
    void fill(FinanceData& data);
}

#endif
