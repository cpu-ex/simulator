#include "sim.h"

static SIM* sim_base;

u32 str2int(char data[33]) {
    u32 val = 0;
    for (int i = 0; i < 32; i++) {
        if (data[31 - i] == '1')
            val |= 1 << i;
    }
    return val;
}

void load_file(char* file_name) {
    char code_name[36], data_name[36];
    sprintf(code_name, "./bin/%s.code", file_name);
    sprintf(data_name, "./bin/%s.data", file_name);
    // read code
    FILE* code = fopen(code_name, "r");
    ADDR addr = 0x10000;
    if (code == NULL) {
        printf("invalid file name: %s.\n", file_name);
        exit(-1);
    } else {
        char buffer[33];
        while (!feof(code)) {
            fgets(buffer, 34, code);
            buffer[32] = '\0';
            if (strlen(buffer) == 32) {
                sim_base->core->store(addr, str2int(buffer), 2);
                addr += 4;
            }
            memset(buffer, 0, 32);
        }
    }
    fclose(code);
    // read data
    FILE* data = fopen(data_name, "r");
    if (data != NULL) {
        char buffer[33];
        while (!feof(data)) {
            fgets(buffer, 34, data);
            buffer[32] = '\0';
            if (strlen(buffer) == 32) {
                sim_base->core->store(addr, str2int(buffer), 2);
                addr += 4;
            }
            memset(buffer, 0, 32);
        }
    }
}

void run() {
    // init GUI
    static WIN win;
    init_win(&win);
    sim_base->win = &win;
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
            BROADCAST(sim_base->win->update(sim_base->core));
            break;
        case STAT_STEP:
            if (!sim_base->core->pc) {
                // pc final return
                BROADCAST(STAT_EXIT);
            } else if ((signed)BROADCAST.decoder.info) {
                sim_base->core->step();
                BROADCAST.decoder.info = (unsigned)((signed)BROADCAST.decoder.info - 1);
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
    // assign interfaces
    sim->load = load_file;
    sim->run = run;
    // broadcast state
    BROADCAST(STAT_HALT);
}
