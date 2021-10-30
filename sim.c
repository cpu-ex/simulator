#include "sim.h"
#include "src/instr.h"

static SIM* sim_base;

ADDR load2mem(char* file_name, ADDR addr, void (*allocate)(u64)) {
    FILE* file = fopen(file_name, "rb");
    if (file == NULL) {
        return 0;
    } else {
        // calculate file size
        fseek(file, 0, SEEK_END);
        u64 size = (u64)ftell(file);
        fseek(file, 0, SEEK_SET);
        // allocate mem
        allocate(size);
        // read file
        u8 byte;
        for (int i = 0; i < size; i++) {
            fread(&byte, 1, 1, file);
            sim_base->core->store(addr++, byte, 0);
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
            } else if ((signed)BROADCAST.decoder.info > 0) {
                // step forward
                sim_base->core->step();
                BROADCAST.decoder.info = (unsigned)((signed)BROADCAST.decoder.info - 1);
            } else if ((signed)BROADCAST.decoder.info < 0) {
                // roll back
                signed remains = (signed)sim_base->core->instr_counter +
                    (signed)BROADCAST.decoder.info;
                sim_base->core->reset();
                u32 steps = (remains < 0) ? 0 : (unsigned)remains;
                BROADCAST(STAT_STEP | ((u64)steps << 32));
            } else {
                BROADCAST(STAT_HALT);
            }
            break;
        default:
            break;
        }
    }
}

void init_sim(SIM* sim) {
    sim_base = sim;
    // init core
    static CORE core;
    init_core(&core);
    sim->core = &core;
    // assign interfaces
    sim->load = load_file;
    sim->run = run;
    // broadcast state
    BROADCAST(STAT_HALT);
}
