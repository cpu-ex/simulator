#include "sim.h"

static SIM* sim_base;

// main loop of simulator
void run() {
    sim_base->win->show_splash_info();
    while (sim_base->state != QUIT) {
        sim_base->state = sim_base->win->update(sim_base->core);
        sim_base->core->if_dc_ex();
    }
    sim_base->win->deinit();
}

void init_sim(SIM* sim, ADDR pc) {
    sim_base = sim;

    sim->state = NEXT;

    static CORE core;
    init_core(&core, pc);
    sim->core = &core;

    static WIN win;
    init_win(&win);
    sim->win = &win;
    
    sim->run = run;
}
