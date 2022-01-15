#pragma once
#include "global.h"

// customizable variables
// #define BP_AT // always taken
// #define BP_NT // always untaken
#define BP_2BIT
// #define BP_BIMODAL
// #define BP_GSHARE
// #define PHT_SIZE 1024

typedef struct branch_predictor {
    #if defined(BP_2BIT)
    u8 counter;
    #elif (defined(BP_BIMODAL)) || (defined(BP_GSHARE))
    // 0 : strongly untaken
    // 1 : weakly untaken
    // 2 : weakly taken
    // 3 : strongly taken
    u8 pht[PHT_SIZE]; // pattern history table
    u32 gh; // global history
    #endif
    u32 hit_counter;
    u32 miss_counter;
    const u32 (*get_branch_stall)(struct branch_predictor* const, const ADDR, const u32);
    void (*reset)(struct branch_predictor*);
} BRANCH_PREDICTOR;

void init_branch_predictor(BRANCH_PREDICTOR* branch_predictor);
