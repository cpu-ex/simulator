#include "sim.h"

static SIM* sim_base;

void run() {
    // main loop of simulator
    while (1) {
        switch (sim_base->state.info.type) {
        case STAT_QUIT:
            sim_base->win->deinit();
            return;
        case STAT_STEP:
            if (sim_base->state.info.steps > 0) {
                sim_base->core->if_dc_ex();
                sim_base->state.info.steps -= 1;
            } else {
                sim_base->state.raw = STAT_HALT;
            }
            break;
        case STAT_HALT:
            sim_base->state.raw = sim_base->win->update(sim_base->core);
            break;
        default:
            break;
        }
    }
}

void init_sim(SIM* sim, ADDR pc) {
    sim_base = sim;

    sim->state.raw = STAT_HALT;

    static CORE core;
    init_core(&core, pc);
    sim->core = &core;

    static WIN win;
    init_win(&win);
    sim->win = &win;
    
    sim->run = run;
}
