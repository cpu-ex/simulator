#pragma once
#include "src/types.h"
#include "src/core.h"
#include "gui/gui.h"

// #define TIME_TEST_MODE

typedef struct sim {
    // attributes
    CORE* core;
    GUI* gui;
    // insterfaces
    void (*load)(struct sim*, char*);
    void (*run)(struct sim*);
} SIM;

void init_sim(SIM* sim);
