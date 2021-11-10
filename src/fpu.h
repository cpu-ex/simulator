#pragma once
#include "types.h"
#include "instr.h"
#include "core.h"

void FLW_EXEC(CORE* core, INSTR instr);
void FSW_EXEC(CORE* core, INSTR instr);
void FMV2F_EXEC(CORE* core, INSTR instr);
void FMV2I_EXEC(CORE* core, INSTR instr);
void FADD_EXEC(CORE* core, INSTR instr);
void FSUB_EXEC(CORE* core, INSTR instr);
void FMUL_EXEC(CORE* core, INSTR instr);
void FDIV_EXEC(CORE* core, INSTR instr);
void FSQRT_EXEC(CORE* core, INSTR instr);
void FCMP_EXEC(CORE* core, INSTR instr);
void FCVT2F_EXEC(CORE* core, INSTR instr);
void FCVT2I_EXEC(CORE* core, INSTR instr);
void FSGNJ_EXEC(CORE* core, INSTR instr);
