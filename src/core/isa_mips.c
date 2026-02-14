#include "core/isa_mips.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

static const InstructionSpec instruction_table[] = {

    {"add", FMT_R, ENC_R_3REG, 0X00, 0x20, 3, {OPK_REG, OPK_REG, OPK_REG}, {ROLE_RD, ROLE_RS, ROLE_RT}, IMM_NONE},
    {"addu", FMT_R, ENC_R_3REG, 0x00, 0x21, 3, {OPK_REG, OPK_REG, OPK_REG}, {ROLE_RD, ROLE_RS, ROLE_RT}, IMM_NONE},
    {"sub", FMT_R, ENC_R_3REG, 0x00, 0x22, 3, {OPK_REG, OPK_REG, OPK_REG}, {ROLE_RD, ROLE_RS, ROLE_RT}, IMM_NONE},
    {"subu", FMT_R, ENC_R_3REG, 0x00, 0x23, 3, {OPK_REG, OPK_REG, OPK_REG}, {ROLE_RD, ROLE_RS, ROLE_RT}, IMM_NONE},
    {"and", FMT_R, ENC_R_3REG, 0x00, 0x24, 3, {OPK_REG, OPK_REG, OPK_REG}, {ROLE_RD, ROLE_RS, ROLE_RT}, IMM_NONE},
    {"or", FMT_R, ENC_R_3REG, 0x00, 0x25, 3, {OPK_REG, OPK_REG, OPK_REG}, {ROLE_RD, ROLE_RS, ROLE_RT}, IMM_NONE},
    {"xor", FMT_R, ENC_R_3REG, 0X00, 0x26, 3, {OPK_REG, OPK_REG, OPK_REG}, {ROLE_RD, ROLE_RS, ROLE_RT}, IMM_NONE},
    {"nor", FMT_R, ENC_R_3REG, 0x00, 0x27, 3, {OPK_REG, OPK_REG, OPK_REG}, {ROLE_RD, ROLE_RS, ROLE_RT}, IMM_NONE},
    {"slt", FMT_R, ENC_R_3REG, 0x00, 0x2A, 3, {OPK_REG, OPK_REG, OPK_REG}, {ROLE_RD, ROLE_RS, ROLE_RT}, IMM_NONE},
    {"sltu", FMT_R, ENC_R_3REG, 0x00, 0x2B, 3, {OPK_REG, OPK_REG, OPK_REG}, {ROLE_RD, ROLE_RS, ROLE_RT}, IMM_NONE},
    {"sllv", FMT_R, ENC_R_3REG, 0x00, 0x04, 3, {OPK_REG, OPK_REG, OPK_REG}, {ROLE_RD, ROLE_RT, ROLE_RS}, IMM_NONE},
    {"srlv", FMT_R, ENC_R_3REG, 0x00, 0x06, 3, {OPK_REG, OPK_REG, OPK_REG}, {ROLE_RD, ROLE_RT, ROLE_RS}, IMM_NONE},
    {"srav", FMT_R, ENC_R_3REG, 0x00, 0x07, 3, {OPK_REG, OPK_REG, OPK_REG}, {ROLE_RD, ROLE_RT, ROLE_RS}, IMM_NONE},

    {"sll", FMT_R, ENC_R_SHIFT_IMM, 0x00, 0x00, 3, {OPK_REG, OPK_REG, OPK_IMM}, {ROLE_RD, ROLE_RT, ROLE_SHAMT}, IMM_SHAMT5},
    {"srl", FMT_R, ENC_R_SHIFT_IMM, 0x00, 0x02, 3, {OPK_REG, OPK_REG, OPK_IMM}, {ROLE_RD, ROLE_RT, ROLE_SHAMT}, IMM_SHAMT5},
    {"sra", FMT_R, ENC_R_SHIFT_IMM, 0x00, 0x03, 3, {OPK_REG, OPK_REG, OPK_IMM}, {ROLE_RD, ROLE_RT, ROLE_SHAMT}, IMM_SHAMT5},


    {"addi", FMT_I, ENC_I_ALU, 0x08, 0x00, 3,{OPK_REG, OPK_REG, OPK_IMM}, {ROLE_RT, ROLE_RS, ROLE_IMM}, IMM_SIGNED16},
    {"addiu", FMT_I, ENC_I_ALU, 0x09, 0x00, 3, {OPK_REG, OPK_REG, OPK_IMM}, {ROLE_RT, ROLE_RS, ROLE_IMM}, IMM_SIGNED16},
    {"andi", FMT_I, ENC_I_ALU, 0x0C, 0x00, 3, {OPK_REG, OPK_REG, OPK_IMM}, {ROLE_RT, ROLE_RS, ROLE_IMM}, IMM_UNSIGNED16},
    {"ori", FMT_I, ENC_I_ALU, 0x0D, 0x00, 3, {OPK_REG, OPK_REG, OPK_IMM}, {ROLE_RT, ROLE_RS, ROLE_IMM}, IMM_UNSIGNED16},
    {"xori", FMT_I, ENC_I_ALU, 0x0E, 0x00, 3, {OPK_REG, OPK_REG, OPK_IMM}, {ROLE_RT, ROLE_RS, ROLE_IMM}, IMM_UNSIGNED16},
    {"slti", FMT_I, ENC_I_ALU, 0x0A, 0x00, 3, {OPK_REG, OPK_REG, OPK_IMM}, {ROLE_RT, ROLE_RS, ROLE_IMM}, IMM_SIGNED16},
    {"sltiu", FMT_I, ENC_I_ALU, 0x0B, 0x00, 3, {OPK_REG, OPK_REG, OPK_IMM}, {ROLE_RT, ROLE_RS, ROLE_IMM}, IMM_SIGNED16},

    {"lw", FMT_I, ENC_I_MEM, 0x23, 0x00, 2, {OPK_REG, OPK_MEM}, {ROLE_RT, ROLE_MEM}, IMM_SIGNED16},  // ops[] and role[] arrays size are 3, but we assumed lw and sw have 2 operand which are OP_REG and OP_MEM(offset and base register) . For security i decided to put OPK_REG and ROLE_NONE into last member of these arrays.
    {"sw", FMT_I, ENC_I_MEM, 0x2B, 0x00, 2, {OPK_REG, OPK_MEM}, {ROLE_RT, ROLE_MEM}, IMM_SIGNED16},
    
    {"beq", FMT_I, ENC_I_BRANCH, 0x04, 0x00, 3, {OPK_REG, OPK_REG, OPK_LABEL}, {ROLE_RS, ROLE_RT, ROLE_LABEL}, IMM_BRANCH16},
    {"bne", FMT_I, ENC_I_BRANCH, 0x05, 0x00, 3, {OPK_REG, OPK_REG, OPK_LABEL}, {ROLE_RS, ROLE_RT, ROLE_LABEL}, IMM_BRANCH16},
    
    {"j", FMT_J, ENC_J_LABEL, 0x02, 0x00, 1, {OPK_LABEL}, {ROLE_LABEL}, IMM_J26},
    {"jal", FMT_J, ENC_J_LABEL, 0x03, 0x00, 1, {OPK_LABEL}, {ROLE_LABEL}, IMM_J26}

};


const InstructionSpec *isa_lookup(const char *mnemonic){

    for(size_t i = 0; i < sizeof(instruction_table) / sizeof(instruction_table[0]); i++){

        if(strcmp(mnemonic, instruction_table[i].mnemonic) == 0) return &instruction_table[i];

    }
    
    return NULL;

}

static OpKind convert_operand_class_to_operand_enum(const OperandClass operand_class){

    switch (operand_class) {
        
        case OPK_REG: return OP_REGISTER;
        case OPK_IMM: return OP_IMMEDIATE;
        case OPK_MEM: return OP_MEMORY;
        case OPK_LABEL: return OP_LABEL;
        default: return OP_LABEL;      // in order to prevent UNDEFINED BEHAVIOUR we add a default case that returns arbitrary enum value.
    }

}

int are_operands_valid(const InstructionSpec *instr_spec, const Operand *ops, size_t op_count){

    if(!instr_spec || !ops) return 0;
    if(instr_spec->op_count != op_count) return 0; 


    for(size_t i = 0; i < op_count; i++){

        if(convert_operand_class_to_operand_enum(instr_spec->ops[i]) != ops[i].kind) return 0;  // one operand of operands is invalid.

    }

    return 1;  // all operands are valid for corresponding instruction.
}