#pragma once
#include "global.h"
#include "instr.h"
#include "core.h"

void FADD_EXEC(CORE* const core, const INSTR instr);
void FSUB_EXEC(CORE* const core, const INSTR instr);
void FMUL_EXEC(CORE* const core, const INSTR instr);
void FDIV_EXEC(CORE* const core, const INSTR instr);
void FSQRT_EXEC(CORE* const core, const INSTR instr);
void FCMP_EXEC(CORE* const core, const INSTR instr);
void FCVT2F_EXEC(CORE* const core, const INSTR instr);
void FCVT2I_EXEC(CORE* const core, const INSTR instr);
void FSGNJ_EXEC(CORE* const core, const INSTR instr);
void init_fpu(void);
