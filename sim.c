#include "sim.h"

static SIM* sim_base;

void run() {
    // main loop of simulator
    while (1) {
        switch (BROADCAST.decoder.type) {
        case STAT_QUIT:
            sim_base->win->deinit();
            return;
        case STAT_EXIT:
        case STAT_MEM_EXCEPTION:
            BROADCAST(sim_base->win->update(sim_base->core));
            if (BROADCAST.decoder.type != STAT_QUIT)
                BROADCAST(STAT_EXIT);
            break;
        case STAT_HALT:
            if (!sim_base->core->pc)
                BROADCAST(STAT_EXIT);
            else
                BROADCAST(sim_base->win->update(sim_base->core));
            break;
        case STAT_STEP:
            if (BROADCAST.decoder.info > 0) {
                sim_base->core->step();
                BROADCAST.decoder.info -= 1;
            } else {
                BROADCAST(STAT_HALT);
            }
            break;
        default:
            break;
        }
    }
}

void init_sim(SIM* sim, ADDR pc) {
    sim_base = sim;
    // init core
    static CORE core;
    init_core(&core, pc);
    sim->core = &core;
    // init GUI
    static WIN win;
    init_win(&win);
    sim->win = &win;
    // assign interfaces
    sim->run = run;
    // broadcast state
    BROADCAST(STAT_HALT);
}
