#pragma once
#include "src/global.h"
#include "src/core.h"
#include "gui/gui.h"

typedef struct sim {
    // attributes
    CORE* core;
    GUI* gui;
    // insterfaces
    void (*load)(struct sim*, char*, char*, char*); // code, data, sld
    void (*run)(struct sim* const);
} SIM;

void init_sim(SIM* sim, u8 is_lite, u8 is_nocache);
