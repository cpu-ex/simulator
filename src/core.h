#pragma once
#include "types.h"
#include "mmu.h"
#include "branch_predictor.h"

#define CLK_FREQUENCY 1000000
#define DEFAULT_PC    0x100
#define UART_ADDR     0x3FFFFFC
#define UART_BUF_SIZE 0x32000 // 200KB

static char* reg_name[32] = {
    "zero",
    "ra",
    "sp",
    "hp",
    "tp",
    "t0", "t1", "t2",
    "fp", "s1",
    "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
    "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",
    "t3", "t4", "t5", "t6"
};

static char* freg_name[32] = {
    "ft0", "ft1", "ft2", "ft3", "ft4", "ft5", "ft6", "ft7",
    "fs0", "fs1",
    "fa0", "fa1", "fa2", "fa3", "fa4", "fa5", "fa6", "fa7",
    "fs2", "fs3", "fs4", "fs5", "fs6", "fs7", "fs8", "fs9", "fs10", "fs11",
    "ft8", "ft9", "ft10", "ft11"
};

typedef struct uart_queue {
    u32 left, right;
    u8* buffer;
    void (*push)(struct uart_queue*, u8);
    u8 (*pop)(struct uart_queue*);
    u8 (*isempty)(struct uart_queue*);
} UART_QUEUE;

void init_uart_queue(UART_QUEUE* uart);

typedef struct core {
    // attributes
    WORD pc;
    WORD regs[32];
    WORD fregs[32];
    UART_QUEUE* uart;
    MMU* mmu;
    BRANCH_PREDICTOR* branch_predictor;
    // output files
    char outputfile_name[30], dumpfile_name[30];
    FILE *outputfile_fp, *dumpfile_fp;
    // analysis
    u64 instr_counter;
    u64 stall_counter;
    u64 instr_analysis[23];
    // interfaces
    WORD (*load_instr)(struct core*, ADDR);
    WORD (*load_data)(struct core*, ADDR, u8 ,u8);
    void (*store_instr)(struct core*, ADDR, WORD);
    void (*store_data)(struct core*, ADDR, WORD, u8);
    void (*step)(struct core*);
    void (*dump)(struct core*);
    void (*reset)(struct core*);
    void (*deinit)(struct core*);
} CORE;

void init_core(CORE* core);
