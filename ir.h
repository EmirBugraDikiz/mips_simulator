#ifndef IR_H
#define IR_H

#include "error_handling.h"
#include <stdint.h>
#include <stdlib.h>

typedef enum{

    SEC_NONE = 0,
    SEC_TEXT,
    SEC_DATA

}Section;


typedef enum{

    OP_REGISTER = 0,
    OP_IMMEDIATE,
    OP_LABEL,
    OP_MEMORY

}OpKind;


typedef struct{

    OpKind kind;
    union{

        int reg;
        int32_t imm;
        char *label;
        struct{

            int32_t offset;
            int base_reg;

        }mem;
    }v;


}Operand;


typedef enum{

    ST_DIR_TEXT = 0,
    ST_DIR_DATA,
    ST_DIR_WORD,
    ST_INSTR,
    ST_LABEL,
    ST_EMPTY,
    ST_LABEL_PLUS_INSTR,
    ST_LABEL_PLUS_DIR_WORD

}StatementKind;


typedef struct{

    int line_no;
    StatementKind kind;
    union{

        struct{}dir_text;
        struct{}dir_data;

        struct{

            int32_t *values;
            size_t n;

        }dir_word;

        struct{

            char mnemonic[16];
            Operand ops[3];
            int op_count;

        }instr;

        struct{

            char name[64];

        }label;

        struct{

            char name[64];
            struct{

                char mnemonic[16];
                Operand ops[3];
                int op_count;

            }instr;

        }label_plus_instr;


        struct{

            char name[64];
            struct{

                int32_t *values;
                size_t n;

            }dir_word;

        }label_plus_dir_word;

    }as;

}Statement;


typedef struct{

    Statement *v;
    size_t n;
    size_t cap;

}IR;


Err ir_init(IR *ir, app_context *app_context_param);
Err ir_push(IR *ir, const Statement *s, app_context *app_context_param);
Err ir_free(IR *ir, app_context *app_context_param);

void operand_free(Operand *op);
void stmt_free_heap_parts(Statement *s);

#endif