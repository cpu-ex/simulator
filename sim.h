#pragma once
#include "src/types.h"
#include "src/core.h"
#include "gui/win.h"

typedef struct sim {
    // attributes
    CORE* core;
    WIN* win;
    // insterfaces
    void (*load)(char*);
    void (*run)(void);
} SIM;

void init_sim(SIM* sim);
