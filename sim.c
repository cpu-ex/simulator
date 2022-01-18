#include "sim.h"

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

void sim_load_file(SIM* sim, char* code_name, char* data_name, char* sld_name) {
    // check before actually loading
    printf("Loading code: %s\n", code_name);
    printf("Loading data: %s (which might be a default value)\n", data_name);
    printf("Loading sld : %s (it's ok to be empty)\n", sld_name);
    printf("Are you sure to proceed? [y]/n ");
    if (getchar() == 'n') exit(0);
    printf("All confirmed.\n");
    // load instr
    u64 file_size = get_file_size(code_name) >> 2;
    if (file_size) {
        sim->core->mmu->allocate_instr(sim->core->mmu, (DEFAULT_PC >> 2) + file_size);
        FILE* file = fopen(code_name, "rb");
        u32 word = 0;
        ADDR addr = DEFAULT_PC >> 2;
        for (int i = 0; i < file_size; ++i) {
            fread(&word, 1, 4, file);
            format2big(word);
            sim->core->mmu->write_instr(sim->core->mmu, addr++, word);
        }
        fclose(file);
    } else {
        printf("invalid code name: %s.\n", code_name);
        exit(-1);
    }
    // load data
    file_size = get_file_size(data_name);
    if (file_size) {
        FILE* file = fopen(data_name, "rd");
        u32 word = 0;
        for (ADDR addr = 0; addr < file_size; addr += 4) {
            fread(&word, 4, 1, file);
            format2big(word);
            sim->core->mmu->data_mem->write_word(sim->core->mmu->data_mem, addr, word);
        }
    }
    // load sld
    file_size = get_file_size(sld_name);
    if (file_size) {
        FILE* file = fopen(sld_name, "rb");
        u8 byte = 0;
        for (int i = 0; i < UART_IN_SIZE && i < file_size; ++i) {
            fread(&byte, 1, 1, file);
            sim->core->uart_in->push(sim->core->uart_in, byte);
        }
        fclose(file);
    }
}

void sim_run_lite(SIM* const sim) {
    // timer
    clock_t t1, t2;
    CORE* const core = sim->core;
    t1 = clock();
    for (; BROADCAST.decoder.type != STAT_EXIT;)
        core->step(core);
    t2 = clock();
    printf(
        "%llu instructions in %ld clk, %lf per sec\n",
        sim->core->instr_counter,
        t2 - t1,
        (f64)sim->core->instr_counter * CLOCKS_PER_SEC / (f64)(t2 - t1)
    );
    sim->core->deinit(sim->core);
}

void sim_run_gui(SIM* const sim) {
    sim->gui->activate(sim->gui);
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
                s64 remains = (s64)sim->core->instr_counter + BROADCAST.decoder.info;
                sim->core->reset(sim->core);
                s64 steps = max(0, remains);
                BROADCAST(STAT_STEP | ((u64)steps << STAT_SHIFT_AMOUNT));
            } else {
                BROADCAST(STAT_HALT);
            }
            break;
        case STAT_HALT:
            BROADCAST(sim->gui->update(sim->gui, sim->core));
            break;
        case STAT_EXIT:
        case STAT_MEM_EXCEPTION:
        case STAT_INSTR_EXCEPTION:
            BROADCAST(sim->gui->update(sim->gui, sim->core));
            if (BROADCAST.decoder.type != STAT_QUIT)
                BROADCAST(STAT_EXIT);
            break;
        case STAT_DUMPING:
            if (BROADCAST.decoder.info > 0) {
                // step forward and record (almost same with STAT_STEP)
                sim->core->step(sim->core);
                sim->core->dump(sim->core);
                BROADCAST.decoder.info--;
            } else {
                // ignore negative step number
                BROADCAST(STAT_HALT);
            }
            break;
        case STAT_QUIT:
            sim->core->deinit(sim->core);
            sim->gui->deinit(sim->gui);
            return;
        default:
            break;
        }
    }
}

void init_sim(SIM* sim, u8 is_lite, u8 is_nocache) {
    // init mmu
    static MMU mmu;
    init_mmu(&mmu, is_lite || is_nocache);
    // init branch predictor
    static BRANCH_PREDICTOR branch_predictor;
    init_branch_predictor(&branch_predictor);
    // init uart queue
    static UART_QUEUE uart_in, uart_out;
    init_uart_queue(&uart_in, UART_IN_SIZE);
    init_uart_queue(&uart_out, UART_OUT_SIZE);
    // init core
    static CORE core;
    init_core(&core);
    sim->core = &core;
    sim->core->mmu = &mmu;
    sim->core->branch_predictor = &branch_predictor;
    sim->core->uart_in = &uart_in;
    sim->core->uart_out = &uart_out;
    // init GUI
    if (!is_lite) {
        static GUI gui;
        init_gui(&gui);
        sim->gui = &gui;
    }
    // assign interfaces
    sim->load = sim_load_file;
    sim->run = is_lite ? sim_run_lite : sim_run_gui;
    // broadcast state
    BROADCAST(STAT_HALT);
}
