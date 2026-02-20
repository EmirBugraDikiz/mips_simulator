#ifndef PASS2_H
#define PASS2_H

#include <stdint.h>
#include <stdlib.h>
#include "asm/pass1.h"
#include "core/error_handling.h"
#include "core/isa_mips.h"

typedef struct{

    uint32_t text_base;
    uint32_t data_base;
    uint32_t *text_image;
    size_t text_words;
    uint32_t *data_image;
    size_t data_words;

}AssemblerOutput;


Err assembler_output_free(AssemblerOutput *out, app_context *app_context_param);

Err assemble_pass2(app_context *app_context_param, const AsmConfig *cfg, const AsmState *final_state, const IR *ir, const Symtab *symtab, AssemblerOutput *out);

Err encode_instruction(app_context *app_context_param, const InstructionSpec *instr_spec, const Operand *ops, const Symtab *symtab, uint32_t current_addr, uint32_t *out_word);

#endif