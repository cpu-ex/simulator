#pragma once
#include "global.h"
#include "instr.h"
#include "core.h"

void FADD_EXEC(CORE* core, INSTR instr);
void FSUB_EXEC(CORE* core, INSTR instr);
void FMUL_EXEC(CORE* core, INSTR instr);
void FDIV_EXEC(CORE* core, INSTR instr);
void FSQRT_EXEC(CORE* core, INSTR instr);
void FCMP_EXEC(CORE* core, INSTR instr);
void FCVT2F_EXEC(CORE* core, INSTR instr);
void FCVT2I_EXEC(CORE* core, INSTR instr);
void FSGNJ_EXEC(CORE* core, INSTR instr);
void init_fpu(void);
