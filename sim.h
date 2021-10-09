#pragma once
#include "types.h"
#include "core.h"
#include "win.h"

typedef struct sim {
    CORE* core;
    WIN* win;

    STATE state;

    void (*run)(void);
} SIM;

void init_sim(SIM* sim, ADDR pc);
