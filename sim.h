#pragma once
#include "src/types.h"
#include "src/core.h"
#include "gui/gui.h"

typedef struct sim {
    // attributes
    CORE* core;
    GUI* gui;
    // insterfaces
    void (*load)(char*);
    void (*run)(void);
} SIM;

void init_sim(SIM* sim);
