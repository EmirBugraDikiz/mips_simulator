#include "asm/pass2.h"
#include "asm/pass1.h"
#include "core/error_handling.h"
#include "core/ir.h"
#include "core/isa_mips.h"
#include "core/symtab.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static uint32_t pack_r(uint8_t rs, uint8_t rt, uint8_t rd, uint8_t shamt, uint8_t funct){

    return ((uint32_t)0u << 26) | ((uint32_t)rs << 21) | ((uint32_t)rt << 16)
     | ((uint32_t)rd << 11) | ((uint32_t)shamt << 6) | ((uint32_t)funct << 0);

}


static uint32_t pack_i(uint8_t opcode, uint8_t rs, uint8_t rt, uint16_t imm){

    return ((uint32_t)opcode << 26) | ((uint32_t)rs << 21)
     | ((uint32_t)rt << 16) | ((uint32_t)imm);

}


static uint32_t pack_j(uint8_t opcode, uint32_t target26){

    return ((uint32_t)opcode << 26) | (target26 & 0x03FFFFFFu);

}


static int check_range_signed16(int32_t value){

    return value >= INT16_MIN && value <= INT16_MAX;

}


static int check_range_unsigned16(int32_t value){

    return value >= 0 && value <= UINT16_MAX;

}

static int check_range_shamt5(int32_t value){

    return value >= 0 && value <= 31;

}

Err assembler_output_free(AssemblerOutput *out, app_context *app_context_param){

    (void)app_context_param; 

    if(!out) return ERR_INVALID_ARGUMENT;

    free(out->text_image);
    free(out->data_image);

    memset(out, 0, sizeof(*out));

    return ERR_OK;

}


static Err encode_r_3reg(app_context *app_context_param, const InstructionSpec *instr_spec, const Operand *ops, uint32_t *out_word){

    (void)app_context_param;

    uint8_t rs = 0;
    uint8_t rt = 0;
    uint8_t rd = 0;
    uint8_t shamt = 0;

    for(size_t i = 0; i < instr_spec->op_count; i++){

        OpRole role = instr_spec->roles[i];

        if(role == ROLE_RS) rs = (uint8_t)ops[i].v.reg;
        
        else if(role == ROLE_RT) rt = (uint8_t)ops[i].v.reg;

        else if(role == ROLE_RD) rd = (uint8_t)ops[i].v.reg;

        else return ERR_SYNTAX;
         
    }

    *out_word = pack_r(rs, rt, rd, shamt, instr_spec->funct);
    return ERR_OK;

}

static Err encode_r_shift_imm(app_context *app_contex_param, const InstructionSpec *instr_spec, const Operand *ops, uint32_t *out_word){

    (void)app_contex_param;

    uint8_t rt = 0;
    uint8_t rd = 0;
    uint32_t shamt = 0;

    for(size_t i = 0; i < instr_spec->op_count; i++){

        OpRole role = instr_spec->roles[i];

        if(role == ROLE_RD) rd = (uint8_t)ops[i].v.reg;
        else if(role == ROLE_RT) rt = (uint8_t)ops[i].v.reg;
        else if(role == ROLE_SHAMT) shamt = ops[i].v.imm;
        else return ERR_SYNTAX;
    }

    if(instr_spec->imm_kind != IMM_SHAMT5) return ERR_SYNTAX;
    if(!check_range_shamt5(shamt)) return ERR_SYNTAX;

    uint8_t shamt5 = (uint8_t)(int8_t)shamt;

    *out_word = pack_r(0, rt, rd, shamt5, instr_spec->funct);
    return ERR_OK;

}

static Err encode_i_alu(app_context *app_context_param, const InstructionSpec *instr_spec, const Operand *ops, uint32_t *out_word){


    (void)app_context_param;

    uint8_t rs = 0;
    uint8_t rt = 0;
    uint32_t imm32 = 0;


    for(size_t i = 0; i < instr_spec->op_count; i++){

        OpRole role = instr_spec->roles[i];

        if(role == ROLE_RS) rs = (uint8_t)ops[i].v.reg;

        else if(role == ROLE_RT) rt = (uint8_t)ops[i].v.reg;

        else if(role == ROLE_IMM) imm32 = ops[i].v.imm;

        else return ERR_SYNTAX;

    }

    uint16_t imm16 = 0;

    switch (instr_spec->imm_kind) {

        case IMM_SIGNED16:
            if(!check_range_signed16(imm32)) return ERR_SYNTAX;
            imm16 = (uint16_t)(int16_t)imm32;
            break;
    
        case IMM_UNSIGNED16:
            if(!check_range_unsigned16(imm32)) return ERR_SYNTAX;
            imm16 = (uint16_t)imm32;
            break;

        default:
            return ERR_SYNTAX;
    }


    *out_word = pack_i(instr_spec->opcode, rs, rt, imm16);
    return ERR_OK;

}


static Err encode_i_mem(app_context *app_context_param, const InstructionSpec *instr_spec, const Operand *ops, uint32_t *out_word){

    (void)app_context_param;

    uint8_t rs = 0;
    uint8_t rt = 0;
    int32_t off = 0;

    for(size_t i = 0; i < instr_spec->op_count; i++){

        OpRole role = instr_spec->roles[i];

        if(role == ROLE_RT) rt = (uint8_t)ops[i].v.reg;

        else if(role == ROLE_MEM){

            rs = (uint8_t)ops[i].v.mem.base_reg;
            off = ops[i].v.mem.offset;

        } 

        else return ERR_SYNTAX;

    }

    if(instr_spec->imm_kind != IMM_SIGNED16) return ERR_SYNTAX;
    if(!check_range_signed16(off)) return ERR_SYNTAX;

    uint16_t imm16 = (uint16_t)(int16_t)off;

    *out_word = pack_i(instr_spec->opcode, rs, rt, imm16);
    return ERR_OK;
    
}


static Err encode_i_branch(app_context *app_context_param, const InstructionSpec *instr_spec, const Operand *ops, const Symtab *symtab, uint32_t current_addr, uint32_t *out_word){

    (void)app_context_param;

    uint8_t rs = 0;
    uint8_t rt = 0;
    const char *label = NULL;


    for(size_t i = 0; i < instr_spec->op_count; i++){

        OpRole role = instr_spec->roles[i];

        if(role == ROLE_RS) rs = (uint8_t)ops[i].v.reg;

        else if(role == ROLE_RT) rt = (uint8_t)ops[i].v.reg;

        else if(role == ROLE_LABEL) label = ops[i].v.label;

        else return ERR_SYNTAX;

    }


    if(!label) return ERR_SYNTAX;

    Symbol sym;

    Err e = symtab_lookup(symtab, label, &sym, app_context_param);

    if(e != ERR_OK) return e;

    if(sym.section != SEC_TEXT){

        APP_ERROR(app_context_param, "branch instruction target must be in .text section");
        return ERR_SYNTAX;

    }

    uint32_t next = current_addr + 4u;
    int32_t delta = (int32_t)sym.addr - (int32_t)next;  // symbol address must be lower than INT32_MAX (0X7FFFFFFF)

    if((delta % 4) != 0) return ERR_SYNTAX;

    int32_t offset_words = delta / 4;

    if(!check_range_signed16(offset_words)) return ERR_SYNTAX;

    uint16_t imm16 = (uint16_t)(int16_t)offset_words;
    *out_word = pack_i(instr_spec->opcode, rs, rt, imm16);
    return ERR_OK;

}


static Err encode_j_label(app_context *app_context_param, const InstructionSpec *instr_spec, const Operand *ops, const Symtab *symtab, uint32_t current_addr, uint32_t *out_word){

    (void)app_context_param;

    const char *label = NULL;

    for(size_t i = 0; i < instr_spec->op_count; i++){

        OpRole role = instr_spec->roles[i];
        if(role == ROLE_LABEL) label = ops[i].v.label;
        else return ERR_SYNTAX;

    }

    if(!label) return ERR_SYNTAX;

    Symbol sym;
    Err e = symtab_lookup(symtab, label, &sym, app_context_param);

    if(e != ERR_OK) return e;


    if((sym.addr & 3u) != 0u){    // is address divisible by 4

        APP_ERROR(app_context_param, "jump target is not 4-byte aligned");
        return ERR_SYNTAX;

    }

    if(sym.section != SEC_TEXT){
        
        APP_ERROR(app_context_param, "jump target must be in .text section");
        return ERR_SYNTAX;

    }
    uint32_t next = current_addr + 4u;

    if((sym.addr & 0xF0000000u) != (next & 0xF0000000u)){  // check target address and PC are in the same 256 mb region.

        APP_ERROR(app_context_param, "jump target out of 256MB region (PC[31:28] mismatch)");
        return ERR_SYNTAX;

    }

    uint32_t target26 = (sym.addr >> 2) & 0x03FFFFFFu;   // calculcate word index with shifting by 2. it is shortly means divide by 4
    *out_word = pack_j(instr_spec->opcode, target26);
    return ERR_OK;

}

Err encode_instruction(app_context *app_context_param, const InstructionSpec *instr_spec, const Operand *ops, const Symtab *symtab, uint32_t current_addr, uint32_t *out_word){

    switch (instr_spec->encoding_kind) {
        
        case ENC_R_3REG: return encode_r_3reg(app_context_param, instr_spec, ops, out_word);
        case ENC_R_SHIFT_IMM: return encode_r_shift_imm(app_context_param, instr_spec, ops, out_word);
        case ENC_I_ALU: return encode_i_alu(app_context_param, instr_spec, ops, out_word);
        case ENC_I_MEM: return encode_i_mem(app_context_param, instr_spec, ops, out_word);
        case ENC_I_BRANCH: return encode_i_branch(app_context_param, instr_spec, ops, symtab, current_addr, out_word);
        case ENC_J_LABEL: return encode_j_label(app_context_param, instr_spec, ops, symtab, current_addr, out_word);
        default: return ERR_SYNTAX;

    }

}

static void stmt_get_instr_view(const Statement *statement, const char **out_mnemonic, const Operand **out_ops, int *out_op_count){

    *out_mnemonic = NULL;
    *out_ops = NULL;
    *out_op_count = 0;

    if(statement->kind == ST_INSTR){

        *out_mnemonic = statement->as.instr.mnemonic;
        *out_ops = statement->as.instr.ops;
        *out_op_count = statement->as.instr.op_count;

    }

    else if(statement->kind == ST_LABEL_PLUS_INSTR){

        *out_mnemonic = statement->as.label_plus_instr.instr.mnemonic;
        *out_ops = statement->as.label_plus_instr.instr.ops;
        *out_op_count = statement->as.label_plus_instr.instr.op_count;

    }

}

static void stmt_get_word_view(const Statement *statement, const int32_t **out_values, size_t *out_n){

    *out_values = NULL;
    *out_n = 0;

    if(statement->kind == ST_DIR_WORD){

        *out_values = statement->as.dir_word.values;
        *out_n = statement->as.dir_word.n;

    }

    else if(statement->kind == ST_LABEL_PLUS_DIR_WORD){

        *out_values = statement->as.label_plus_dir_word.dir_word.values;
        *out_n = statement->as.label_plus_dir_word.dir_word.n;

    }

}


Err assemble_pass2(app_context *app_context_param, const AsmConfig *cfg, const AsmState *final_state, const IR *ir, const Symtab *symtab, AssemblerOutput *out){

    if(!cfg || !final_state || !ir || !symtab || !out) return ERR_INVALID_ARGUMENT;


    memset(out, 0, sizeof(*out));
    out->text_base = cfg->text_base;
    out->data_base = cfg->data_base;

    if((final_state->text_pc % 4) != 0 || (final_state->data_pc % 4) != 0) return ERR_SYNTAX;

    out->text_words = final_state->text_pc / 4;
    out->data_words = final_state->data_pc / 4;

    out->text_image = (out->text_words) ? (uint32_t*)calloc(out->text_words, sizeof(uint32_t)) : NULL;
    out->data_image = (out->data_words) ? (uint32_t*)calloc(out->data_words, sizeof(uint32_t)) : NULL;

    if((out->text_words && !out->text_image) || (out->data_words && !out->data_image)){

        assembler_output_free(out, app_context_param);
        return ERR_OOM;

    }

    Section section = SEC_NONE;

    uint32_t text_pc = 0;
    uint32_t data_pc = 0;

    for(size_t i = 0; i < ir->n; i++){

        fprintf(stderr, "DEBUG: for loop iteration for ir statements\n");

        const Statement *st = &ir->v[i];

        switch(st->kind){

            case ST_DIR_TEXT:
                section = SEC_TEXT;
                break;

            case ST_DIR_DATA:
                section = SEC_DATA;
                break;

            case ST_LABEL:
            case ST_EMPTY:
                // nothing (no output)
                break;

            case ST_DIR_WORD:
            case ST_LABEL_PLUS_DIR_WORD: {

                if(section != SEC_DATA){

                    assembler_output_free(out, app_context_param);
                    return ERR_SYNTAX;
                }

                const int32_t *values = NULL;
                size_t n = 0;
                stmt_get_word_view(st, &values, &n);

                if(data_pc / 4 + n > out->data_words){

                    assembler_output_free(out, app_context_param);
                    return ERR_SYNTAX;

                }

                for(size_t k = 0; k < n; k++){

                    out->data_image[data_pc / 4 + k] = (uint32_t)values[k];

                }

                data_pc += (uint32_t)(4 * n);
            } break;

            case ST_INSTR:
            case ST_LABEL_PLUS_INSTR: {

                if(section != SEC_TEXT){

                    assembler_output_free(out, app_context_param);
                    return ERR_SYNTAX;

                }

                const char *mnemonic = NULL;
                const Operand *ops = NULL;
                int op_count = 0;

                stmt_get_instr_view(st, &mnemonic, &ops, &op_count);

                const InstructionSpec *instr_spec = isa_lookup(mnemonic);

                if(!instr_spec){

                    assembler_output_free(out, app_context_param);
                    return ERR_SYNTAX;

                }

                // parser already handle this stuation but i am using it again as an internal invariant.

                if(!are_operands_valid(instr_spec, ops, (size_t)op_count)){

                    assembler_output_free(out, app_context_param);
                    return ERR_SYNTAX;

                }

                // checking bounds

                if((text_pc / 4) >= out->text_words){

                    assembler_output_free(out, app_context_param);
                    return ERR_SYNTAX;

                }

                uint32_t current_addr = cfg->text_base + text_pc;
                uint32_t word = 0;

                Err e = encode_instruction(app_context_param, instr_spec, ops, symtab, current_addr, &word);
                if(e != ERR_OK){

                    assembler_output_free(out, app_context_param);
                    return e;

                }

                out->text_image[text_pc / 4] = word;
                text_pc += 4;

            } break;
            
            default:
                assembler_output_free(out, app_context_param);
                return ERR_SYNTAX;
        }

    } 

    if(text_pc != final_state->text_pc || data_pc != final_state->data_pc){

        assembler_output_free(out, app_context_param);
        return ERR_SYNTAX;

    }

    return ERR_OK;
}
