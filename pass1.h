#ifndef PASS1_H
#define PASS1_H


#include "ir.h"
#include "symtab.h"

typedef struct{

    Section section;
    uint32_t text_pc;
    uint32_t data_pc;

}AsmState;

typedef struct{

    uint32_t text_base;
    uint32_t data_base;

}AsmConfig;

Err assemble_pass1(app_context *app_context_param, const AsmConfig *config, char **lines, size_t nlines, IR *out_ir, Symtab *out_symtab, AsmState *out_final_state);



#endif