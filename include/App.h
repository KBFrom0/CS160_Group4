#ifndef APP_H
#define APP_H

#include "FinanceData.h"

// Owns the data and runs the main loop:
// load -> process recurring -> [Dashboard + menu -> run feature -> back to Dashboard] -> save.
class App {
private:
    FinanceData data;

    void showDashboard();
    void showNotImplemented(const char* feature);

public:
    void run();
};

#endif
