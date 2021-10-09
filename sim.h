#pragma once
#include "types.h"
#include "core.h"
#include "win.h"

typedef struct sim {
    CORE* core;
    WIN* win;

    void (*run)(void);
} SIM;

void init_sim(SIM* sim);
