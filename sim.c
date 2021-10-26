#include "sim.h"
#include "src/instr.h"

static SIM* sim_base;

ADDR load2mem(char* file_name, ADDR addr, void (*allocate)(u64)) {
    FILE* file = fopen(file_name, "r");
    if (file == NULL) {
        return 0;
    } else {
        // calculate line num
        fseek(file, 0, SEEK_END);
        u64 len = ((u64)ftell(file) + 1) / 33;
        fseek(file, 0, SEEK_SET);
        // allocate mem
        allocate(len * 4);
        // read file
        char buffer[33];
        while (!feof(file)) {
            fgets(buffer, 34, file);
            buffer[32] = '\0';
            if (strlen(buffer) == 32) {
                sim_base->core->store(addr, str2int(buffer), 2);
                addr += 4;
            }
            memset(buffer, 0, 32);
        }
        fclose(file);
        return addr;
    }
}

void load_file(char* file_name) {
    char code_name[36], data_name[36];
    sprintf(code_name, "./bin/%s.code", file_name);
    sprintf(data_name, "./bin/%s.data", file_name);
    // load instr
    ADDR addr = 0x10000;
    addr = load2mem(code_name, addr, sim_base->core->mmu->allocate_instr);
    if (!addr) {
        printf("invalid file name: %s.\n", file_name);
        exit(-1);
    }
    // load data
    addr = load2mem(data_name, addr, sim_base->core->mmu->allocate_data);
    if (!addr) {
        sim_base->core->mmu->allocate_data(0x100);
    }
    // set stack
    sim_base->core->mmu->allocate_stack(0x200);
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
        case STAT_INSTR_EXCEPTION:
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
