#include "sim.h"

static SIM* sim_base;

void run() {
    sim_base->win->wait();
    sim_base->win->deinit();
}

void init_sim(SIM* sim) {
    sim_base = sim;

    static CORE core;
    init_core(&core);
    sim->core = &core;

    static WIN win;
    init_win(&win);
    sim->win = &win;
    
    sim->run = run;
}
