#include "branch_predictor.h"

void bp_log(BRANCH_PREDICTOR* branch_predictor, u8 predicted, u8 result) {
    if ((!predicted) && (!result))
        // both untaken
        branch_predictor->hit_counter++;
    else if ((!predicted) || (!result))
        // either untaken
        branch_predictor->miss_counter++;
    else
        // both taken
        branch_predictor->hit_counter++;
}

u8 bp_predict(BRANCH_PREDICTOR* branch_predictor, ADDR pc, u8 result) {
    u8 predicted;
    #if defined(BP_AT)
    predicted = 1;
    #elif defined(BP_NT)
    predicted = 0;
    #elif defined(BP_2BIT)
    u8 prev_stat = branch_predictor->counter;
    u8 curr_stat = result ? min(prev_stat + 1, 3) : max((s8)prev_stat - 1, 0);
    predicted = (prev_stat < 2) ? 0 : 1;
    branch_predictor->counter = curr_stat;
    #elif defined(BP_BIMODAL)
    u8 prev_stat = branch_predictor->pht[pc % PHT_SIZE];
    u8 curr_stat = result ? min(prev_stat + 1, 3) : max((s8)prev_stat - 1, 0);
    predicted = (prev_stat < 2) ? 0 : 1;
    branch_predictor->pht[pc % PHT_SIZE] = curr_stat;
    #elif defined(BP_GSHARE)
    u8 prev_stat = branch_predictor->pht[(pc ^ branch_predictor->gh) % PHT_SIZE];
    u8 curr_stat = result ? min(prev_stat + 1, 3) : max((s8)prev_stat - 1, 0);
    predicted = (prev_stat < 2) ? 0 : 1;
    branch_predictor->pht[(pc ^ branch_predictor->gh) % PHT_SIZE] = curr_stat;
    branch_predictor->gh = (branch_predictor->gh << 1) | (result ? 1 : 0);
    #else
    predicted = 0; // default to always untaken
    #endif
    bp_log(branch_predictor, predicted, result);
    return predicted;
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
    branch_predictor->predict = bp_predict;
    branch_predictor->reset = bp_reset;
}
