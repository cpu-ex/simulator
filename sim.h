#pragma once
#include "src/types.h"
#include "src/core.h"
#include "gui/win.h"

typedef struct sim {
    CORE* core;
    WIN* win;

    void (*load)(char*);
    void (*run)(void);
} SIM;

void init_sim(SIM* sim, ADDR pc);
