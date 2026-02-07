#include "core/ir.h"
#include "core/error_handling.h"
#include <stdlib.h>


Err ir_init(IR *ir, app_context *app_context_param){

    if(!ir) {

        APP_ERROR(app_context_param, "INVALID ARGUMENT & IR INIT FAILED");
        return ERR_INVALID_ARGUMENT;

    }

    ir->v = NULL;
    ir->n = 0;
    ir->cap = 0;

    return ERR_OK;

}

static Err ir_grow(IR *ir, app_context *app_context_param){

    size_t new_cap = (ir->cap == 0)? 64 : (ir->cap * 2);
    Statement *s_p = realloc(ir->v, sizeof(*s_p) * new_cap);
    
    if(!s_p){

        APP_PERROR(app_context_param, "IR REALLOC FAILED.");
        return ERR_OOM;

    }

    ir->v = s_p;
    ir->cap = new_cap;

    return ERR_OK;

}


Err ir_push(IR *ir, const Statement *s, app_context *app_context_param){

    if(!ir || !s){

        APP_ERROR(app_context_param, "INVALID ARGUMENT");
        return ERR_INVALID_ARGUMENT;

    }

    if(ir->cap == ir->n){

        Err e = ir_grow(ir, app_context_param);
        if(e != ERR_OK) return e;
    }

    ir->v[ir->n++] = *s;

    return ERR_OK;
}

void operand_free(Operand *op){

    if(!op) return;

    if(op->kind == OP_LABEL && op->v.label){

        free(op->v.label);
        op->v.label = NULL;

    }

}


void stmt_free_heap_parts(Statement *s){

    if(!s) return;

    if(s->kind == ST_DIR_WORD){

        free(s->as.dir_word.values);
        s->as.dir_word.values = NULL;
        s->as.dir_word.n = 0;

    }

    else if(s->kind == ST_INSTR){

        for(size_t i = 0; i < (size_t)s->as.instr.op_count; i++){

            operand_free(&s->as.instr.ops[i]);

        }

    }


    else if(s->kind == ST_LABEL_PLUS_DIR_WORD){

        free(s->as.label_plus_dir_word.dir_word.values);
        s->as.label_plus_dir_word.dir_word.values = NULL;
        s->as.label_plus_dir_word.dir_word.n = 0;

    }


    else if(s->kind == ST_LABEL_PLUS_INSTR){

        for(size_t i = 0; i < (size_t)s->as.label_plus_instr.instr.op_count; i++){

            operand_free(&s->as.label_plus_instr.instr.ops[i]);

        }

    }

}


Err ir_free(IR *ir, app_context *app_context_param){

    (void)app_context_param;

    if(!ir) return ERR_INVALID_ARGUMENT;

    for(size_t i = 0; i < ir->n; i++){

        stmt_free_heap_parts(&ir->v[i]);

    }

    free(ir->v);
    ir->v = NULL;
    ir->cap = ir->n = 0;

    return ERR_OK;
}

