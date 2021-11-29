#pragma once
#include "types.h"

// #define BP_AT // always taken
// #define BP_NT // always untaken
#define BP_BIMODAL
#define PHT_SIZE 0x100

typedef struct branch_predictor {
    #if defined(BP_BIMODAL)
    // 0 : strongly untaken
    // 1 : weakly untaken
    // 2 : weakly taken
    // 3 : strongly taken
    u8 pht[PHT_SIZE];
    #endif
    u32 hit_counter;
    u32 miss_counter;
    u8 (*predict)(struct branch_predictor*, ADDR, u8);
    void (*reset)(struct branch_predictor*);
} BRANCH_PREDICTOR;

void init_branch_predictor(BRANCH_PREDICTOR* branch_predictor);
