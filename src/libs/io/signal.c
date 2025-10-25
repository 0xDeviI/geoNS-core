#include "signal.h"

void SIGINT_HANDLER(int sigsum) {
    is_geons_running = 0;
}