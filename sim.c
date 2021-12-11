#include "sim.h"
#include <time.h>

u64 get_file_size(char* file_name) {
    FILE* file = fopen(file_name, "rb");
    if (file == NULL) {
        return 0;
    } else {
        fseek(file, 0, SEEK_END);
        u64 size = (u64)ftell(file);
        fclose(file);
        return size;
    }
}

void sim_load_file(SIM* sim, char* file_name) {
    char code_name[36], data_name[36];
    sprintf(code_name, "./bin/%s.code", file_name);
    sprintf(data_name, "./bin/%s.data", file_name);
    u64 file_size;
    // load instr
    file_size = get_file_size(code_name);
    if (file_size) {
        sim->core->mmu->allocate_instr(sim->core->mmu, 0x100 + file_size);
        FILE* file = fopen(code_name, "rb");
        u8 byte = 0;
        ADDR addr = DEFAULT_PC;
        for (int i = 0; i < file_size; i++) {
            fread(&byte, 1, 1, file);
            sim->core->mmu->write_instr(sim->core->mmu, addr++, byte);
        }
        fclose(file);
    } else {
        printf("invalid file name: %s.\n", file_name);
        exit(-1);
    }
    // load data
    file_size = get_file_size(data_name);
    if (file_size) {
        FILE *file = fopen(data_name, "rb");
        u8 byte = 0;
        ADDR addr = 0;
        for (int i = 0; i < file_size; i++) {
            fread(&byte, 1, 1, file);
            sim->core->mmu->data_mem->write_byte(sim->core->mmu->data_mem, addr++, byte);
        }
        fclose(file);
    }
}

void sim_run(SIM* sim) {
    // init GUI
    static GUI gui;
    init_gui(&gui);
    sim->gui = &gui;
    // timer
    clock_t t1, t2;
    BROADCAST(STAT_STEP | ((u64)0x7FFFFFFF << 32));
    t1 = clock();
    // main loop of simulator
    for (;;) {
        switch (BROADCAST.decoder.type) {
        case STAT_STEP:
            if (BROADCAST.decoder.info > 0) {
                // step forward
                sim->core->step(sim->core);
                BROADCAST.decoder.info--;
            } else if (BROADCAST.decoder.info < 0) {
                // roll back
                signed remains = (signed)sim->core->instr_counter + BROADCAST.decoder.info;
                sim->core->reset(sim->core);
                signed steps = max(0, remains);
                BROADCAST(STAT_STEP | ((u64)steps << 32));
            } else {
                BROADCAST(STAT_HALT);
            }
            break;
        case STAT_EXIT:
            t2 = clock();
            u32 num = sim->core->instr_counter;
            fprintf(stderr, "%u instructions in %ld clk, %lf per sec\n", num, t2 - t1, (double)num * CLOCKS_PER_SEC / (double)(t2 - t1));
            sim->gui->deinit(sim->gui);
            return;
        case STAT_MEM_EXCEPTION:
        case STAT_INSTR_EXCEPTION:
            BROADCAST(sim->gui->update(sim->gui, sim->core));
            if (BROADCAST.decoder.type != STAT_QUIT)
                BROADCAST(STAT_EXIT);
            break;
        case STAT_HALT:
            BROADCAST(sim->gui->update(sim->gui, sim->core));
            break;
        case STAT_QUIT:
            sim->gui->deinit(sim->gui);
            return;
        default:
            break;
        }
    }
}

void init_sim(SIM* sim) {
    // init core
    static CORE core;
    init_core(&core);
    sim->core = &core;
    // assign interfaces
    sim->load = sim_load_file;
    sim->run = sim_run;
    // broadcast state
    BROADCAST(STAT_HALT);
}
