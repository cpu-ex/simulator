#include "branch_predictor.h"

u32 bp_get_branch_stall(BRANCH_PREDICTOR* const branch_predictor, const ADDR pc, const u32 result) {
    register u32 predicted;
    #if defined(BP_AT)
    predicted = 1;
    #elif defined(BP_NT)
    predicted = 0;
    #elif defined(BP_2BIT)
    register const u8 prev_stat = branch_predictor->counter;
    predicted = (prev_stat < 2) ? 0 : 1;
    branch_predictor->counter = result ? min(prev_stat + 1, 3) : max((s8)prev_stat - 1, 0);
    #elif defined(BP_BIMODAL)
    register const u8 prev_stat = branch_predictor->pht[(pc >> 2) % PHT_SIZE];
    predicted = (prev_stat < 2) ? 0 : 1;
    branch_predictor->pht[(pc >> 2) % PHT_SIZE] = result ? min(prev_stat + 1, 3) : max((s8)prev_stat - 1, 0);
    #elif defined(BP_GSHARE)
    register const u8 prev_stat = branch_predictor->pht[((pc >> 2) ^ branch_predictor->gh) % PHT_SIZE];
    predicted = (prev_stat < 2) ? 0 : 1;
    branch_predictor->pht[((pc >> 2) ^ branch_predictor->gh) % PHT_SIZE] = result ? min(prev_stat + 1, 3) : max((s8)prev_stat - 1, 0);
    branch_predictor->gh = (branch_predictor->gh << 1) | (result ? 1 : 0);
    #else
    predicted = 0; // default to always untaken
    #endif
    if (predicted  == result) {
        // hit
        ++branch_predictor->hit_counter;
        return 0;
    } else {
        // miss
        ++branch_predictor->miss_counter;
        return 2;
    }
}

void bp_reset(BRANCH_PREDICTOR* branch_predictor) {
    #if defined(BP_2BIT)
    branch_predictor->counter = 0;
    #elif (defined(BP_BIMODAL)) || (defined(BP_GSHARE))
    memset(branch_predictor->pht, 0, PHT_SIZE * sizeof(u8));
    branch_predictor->gh = 0;
    #endif
    branch_predictor->hit_counter = 0;
    branch_predictor->miss_counter = 0;
}

void init_branch_predictor(BRANCH_PREDICTOR* branch_predictor) {
    bp_reset(branch_predictor);
    // assign interfaces
    branch_predictor->get_branch_stall = bp_get_branch_stall;
    branch_predictor->reset = bp_reset;
}
