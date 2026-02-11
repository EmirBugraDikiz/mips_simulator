#ifndef ISA_MIPS_H
#define ISA_MIPS_H

#include <stdint.h>
#include "ir.h"

typedef enum {

    FMT_R,
    FMT_I,
    FMT_J

}InstructionFormat;


typedef enum{

    OPK_REG,
    OPK_IMM,
    OPK_MEM,
    OPK_LABEL

}OperandClass;

typedef enum{

    IMM_NONE,
    IMM_SIGNED16,
    IMM_UNSIGNED16,
    IMM_BRANCH16,
    IMM_J26,
    IMM_SHAMT5

}ImmKind;

typedef enum{

    ROLE_RS,
    ROLE_RT,
    ROLE_RD,
    ROLE_SHAMT,
    ROLE_IMM,
    ROLE_MEM,
    ROLE_LABEL

}OpRole;

typedef struct{

    const char *mnemonic;
    InstructionFormat format;
    uint8_t opcode;
    uint8_t funct;
    uint8_t op_count;
    OperandClass ops[3];
    OpRole roles[3];
    ImmKind imm_kind;    

}InstructionSpec;

const InstructionSpec *isa_lookup(const char *mnemonic);

int are_operands_valid(const InstructionSpec *instr_spec, const Operand *ops, size_t op_count);

#endif