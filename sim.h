#pragma once
#include "types.h"
#include "core.h"
#include "win.h"

typedef struct sim {
    CORE* core;
    WIN* win;

    union _state {
        STATE raw;
        struct _info {
            u_int8_t type : 3;
            u_int32_t steps: 29;
        } info;
    } state;

    void (*run)(void);
} SIM;

void init_sim(SIM* sim, ADDR pc);
