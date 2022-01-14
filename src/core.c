#include "core.h"
#include "exec.h"
#include "fpu.h"

/******************** uart ********************/

u8 uart_isempty(const UART_QUEUE* uart) {
    return (uart->left < uart->right) ? 0 : 1;
}

void uart_push(UART_QUEUE* const uart, const u8 val) {
    uart->buffer[(uart->right++) % uart->size] = val;
}

u8 uart_pop(UART_QUEUE* const uart) {
    return (uart->left < uart->right) ? uart->buffer[(uart->left++) % uart->size] : 0;
}

void init_uart_queue(UART_QUEUE* uart, u32 size) {
    uart->left = 0;
    uart->right = 0;
    uart->size = size;
    uart->buffer = (u8*)malloc(size * sizeof(u8));
    uart->push = uart_push;
    uart->pop = uart_pop;
    uart->isempty = uart_isempty;
}

/******************** core ********************/

void core_step(CORE* const core) {
    // rudely fetch + decode + execute
    execute(core, (INSTR){ .raw = core->mmu->instr_mem[core->pc >> 2] });
    core->regs[0] = 0;
    ++core->instr_counter;
}

WORD core_load_instr(const CORE* core, const ADDR addr) {
    return core->mmu->read_instr(core->mmu, addr >> 2);
}

WORD core_load_data(const CORE* core, const ADDR addr) {
    return (addr != UART_ADDR) ? core->mmu->read_data(core->mmu, (void* const)core, addr) : core->uart_in->pop(core->uart_in);
}

void core_store_instr(const CORE* core, const ADDR addr, const WORD val) {
    core->mmu->write_instr(core->mmu, addr >> 2, val);
}

void core_store_data(const CORE* core, const ADDR addr, const WORD val) {
    if (addr != UART_ADDR)
        core->mmu->write_data(core->mmu, (void* const)core, addr, val);
    else
        core->uart_out->push(core->uart_out, val & 0xFF);
}

void core_dump(CORE* core) {
    // check dumpfile
    if (core->dumpfile_fp == NULL)
        core->dumpfile_fp = fopen(core->dumpfile_name, "w");
    // step, pc
    fprintf(core->dumpfile_fp, "step:%016llx pc:%08x", core->instr_counter, core->pc); 
    // register file
    for (int i = 0; i < 64; ++i) {
        if (i < 32) {
            fprintf(core->dumpfile_fp, " x%d:%08x", i, core->regs[i]);
        } else {
            fprintf(core->dumpfile_fp, " f%d:%08x", i - 32, core->fregs[i - 32]);
        }
    }
    fprintf(core->dumpfile_fp, "\r\n");
}

void core_reset(CORE* core) {
    // call mem reset
    core->mmu->reset(core->mmu, core->regs[2]);
    // reset registers
    core->pc = DEFAULT_PC;
    memset(core->regs, 0, 32 * sizeof(u32));
    memset(core->fregs, 0, 32 * sizeof(u32));
    // reset uart queue
    core->uart_in->left = 0;
    core->uart_out->right = 0;
    // reset instruction analysis
    core->instr_counter = 0;
    core->stall_counter = 0;
    memset(core->instr_analysis, 0, 23 * sizeof(u64));
    // reset branch predictor
    core->branch_predictor->reset(core->branch_predictor);
}

#define close_file(fp, name)             \
    {                                    \
        if (fp != NULL) {                \
            fseek(fp, 0, SEEK_END);      \
            u64 filesize = ftell(fp);    \
            fclose(fp);                  \
            if (!filesize) remove(name); \
            fp = NULL;                   \
        }                                \
    }
void core_deinit(CORE* core) {
    FILE* outputfile_fp = fopen(core->outputfile_name, "w");
    while (!core->uart_out->isempty(core->uart_out)) {
        u8 byte = core->uart_out->pop(core->uart_out);
        fwrite(&byte, 1, 1, outputfile_fp);
    }
    close_file(outputfile_fp, core->outputfile_name);
    close_file(core->dumpfile_fp, core->dumpfile_name);
}

f64 core_predict_exec_time(CORE* core) {
    f64 code_sending = (f64)core->mmu->instr_len * 40.0 / UART_BAUDRATE;
    f64 instr_executing = (f64)(core->instr_counter + core->stall_counter) / CLK_FREQUENCY;
    return code_sending + instr_executing;
}

void init_core(CORE* core) {
    // init basic info
    core->pc = DEFAULT_PC;
    core->instr_counter = 0;
    core->stall_counter = 0;
    memset(core->instr_analysis, 0, 23 * sizeof(u64));
    // prepare for outputs
    time_t curr_time = time(NULL);
    struct tm* info = localtime(&curr_time);
    strftime(core->outputfile_name, 30, "output-%Y%m%d-%H%M%S", info);
    strftime(core->dumpfile_name, 30, "dumpfile-%Y%m%d-%H%M%S", info);
    // init fpu
    init_fpu();
    // assign interfaces
    core->load_instr = core_load_instr;
    core->load_data = core_load_data;
    core->store_instr = core_store_instr;
    core->store_data = core_store_data;
    core->step = core_step;
    core->dump = core_dump;
    core->reset = core_reset;
    core->deinit = core_deinit;
    core->predict_exec_time = core_predict_exec_time;
}
