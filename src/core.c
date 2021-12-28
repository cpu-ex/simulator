#include "core.h"
#include "exec.h"
#include "fpu.h"

/******************** uart ********************/

u8 uart_isempty(UART_QUEUE* uart) {
    return (uart->left < uart->right) ? 0 : 1;
}

void uart_push(UART_QUEUE* uart, u8 val) {
    uart->buffer[(uart->right++) % UART_BUF_SIZE] = val;
}

u8 uart_pop(UART_QUEUE* uart) {
    return (uart->left < uart->right) ? uart->buffer[(uart->left++) % UART_BUF_SIZE] : 0;
}

void init_uart_queue(UART_QUEUE* uart) {
    uart->left = 0;
    uart->right = 0;
    uart->buffer = (u8*)malloc(UART_BUF_SIZE * sizeof(u8));
    uart->push = uart_push;
    uart->pop = uart_pop;
    uart->isempty = uart_isempty;
}

/******************** core ********************/

void core_step(CORE* core) {
    // fetch and decode
    static INSTR curr_instr;
    curr_instr.raw = core->load_instr(core, core->pc);
    // execute
    execute(core, curr_instr);
    core->regs[0] = 0;
    core->instr_counter++;
}

WORD core_load_instr(CORE* core, ADDR addr) {
    return core->mmu->read_instr(core->mmu, addr >> 2);
}

WORD core_load_data(CORE* core, ADDR addr, u8 bytes, u8 sign) {
    if (addr ^ UART_ADDR) {
        WORD val = 0;
        for (int i = 0; i < (1 << bytes); i++) {
            val <<= 8;
            val |= core->mmu->read_data(core->mmu, addr + i);
        }
        return sign ? sext(val, (1 << bytes) * 8 - 1) : val;
    } else {
        return core->uart_in->pop(core->uart_in);
    }
}

void core_store_instr(CORE* core, ADDR addr, WORD val) {
    core->mmu->write_instr(core->mmu, addr >> 2, val);
}

void core_store_data(CORE* core, ADDR addr, WORD val, u8 bytes) {
    if (addr ^ UART_ADDR) {
        for (int i = (1 << bytes) - 1; i >= 0; i--) {
            core->mmu->write_data(core->mmu, addr + i, val & 0xFF);
            val >>= 8;
        }
    } else {
        core->uart_out->push(core->uart_out, val & 0xFF);
    }
}

void core_dump(CORE* core) {
    // step, pc
    fprintf(core->dumpfile_fp, "step:%016llx pc:%08x", core->instr_counter, core->pc); 
    // register file
    for (int i = 0; i < 64; i++) {
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
    for (int i = 0; i < 32; i++) {
        core->regs[i] = 0;
        core->fregs[i] = 0;
    }
    // reset uart queue
    core->uart_in->left = 0;
    core->uart_out->right = 0;
    // reset instruction analysis
    core->instr_counter = 0;
    core->stall_counter = 0;
    memset(core->instr_analysis, 0, 23 * sizeof(u32));
    // reset branch predictor
    core->branch_predictor->reset(core->branch_predictor);
}

#define close_file(fp, name)         \
    {                                \
        fseek(fp, 0, SEEK_END);      \
        u64 filesize = ftell(fp);    \
        fclose(fp);                  \
        if (!filesize) remove(name); \
        fp = NULL;                   \
    }
void core_deinit(CORE* core) {
    while (!core->uart_out->isempty(core->uart_out)) {
        u8 byte = core->uart_out->pop(core->uart_out);
        fwrite(&byte, 1, 1, core->outputfile_fp);
    }
    close_file(core->outputfile_fp, core->outputfile_name);
    close_file(core->dumpfile_fp, core->dumpfile_name);
}

void init_core(CORE* core) {
    // init basic info
    core->pc = DEFAULT_PC;
    core->instr_counter = 0;
    core->stall_counter = 0;
    memset(core->instr_analysis, 0, 23 * sizeof(u32));
    // open files for outputs
    time_t curr_time = time(NULL);
    struct tm* info = localtime(&curr_time);
    strftime(core->outputfile_name, 30, "output-%Y%m%d-%H%M%S", info);
    strftime(core->dumpfile_name, 30, "dumpfile-%Y%m%d-%H%M%S", info);
    core->outputfile_fp = fopen(core->outputfile_name, "w");
    core->dumpfile_fp = fopen(core->dumpfile_name, "w");
    // init uart queue
    static UART_QUEUE uart_in, uart_out;
    init_uart_queue(&uart_in);
    init_uart_queue(&uart_out);
    core->uart_in = &uart_in;
    core->uart_out = &uart_out;
    // init mmu
    static MMU mmu;
    init_mmu(&mmu);
    core->mmu = &mmu;
    // init branch predictor
    static BRANCH_PREDICTOR branch_predictor;
    init_branch_predictor(&branch_predictor);
    core->branch_predictor = &branch_predictor;
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
}
