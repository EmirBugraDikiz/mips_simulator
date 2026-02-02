#include "pass1.h"
#include "error_handling.h"
#include "symtab.h"
#include "preprocess.h"
#include "lexer.h"
#include "parser.h"
#include <string.h>

Err assembl_pass1(app_context *app_context_param, const AsmConfig *cfg, char **lines, size_t nlines, IR *out_ir, Symtab *out_symtab, AsmState *out_final_state){

    if(!app_context_param || !cfg || !lines || !out_ir || !out_symtab || !out_final_state) {

        APP_ERROR(app_context_param, "INVALID ARGUMENT");
        return ERR_INVALID_ARGUMENT;

    }

    Err e;
    if((e = ir_init(out_ir, app_context_param)) != ERR_OK) return e;
    if ((e = symtab_init(out_symtab, app_context_param)) != ERR_OK) return e;

    AsmState state = {0};
    state.section = SEC_NONE;
    state.text_pc = 0;
    state.data_pc = 0;

    for(size_t ith_line = 0; ith_line < nlines; ith_line++){

        char buf[1024];
        strncpy(buf, lines[ith_line]? lines[ith_line] : "", sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';


        strip_comment(buf);
        trim_inplace(buf);

        if(buf[0] == '\0') continue;

        TokenVec tv;
        e = lex_line(buf, (int)ith_line, &tv, app_context_param);

        if(e != ERR_OK){

            tokenvec_free(&tv, app_context_param);
            ir_free(out_ir, app_context_param);
            symtab_free(out_symtab, app_context_param);
            
            return e;

        }

        
        int has_label = 0;
        Statement statement;

        e = parse_line(app_context_param, &tv, buf, (int)ith_line + 1, &has_label, &statement);
        tokenvec_free(&tv, app_context_param);


        if(e != ERR_OK){

            // statement that acquired from parse_line can be have some heap parts, so we have to check and free it for per line

            stmt_free_heap_parts(&statement);
            ir_free(out_ir, app_context_param);
            symtab_free(out_symtab, app_context_param);

            return e;

        }

        if(statement.kind == ST_LABEL){

            if(state.section == SEC_NONE){

                APP_ERROR(app_context_param, "label before selecting .text/.data section");
                stmt_free_heap_parts(&statement);
                ir_free(out_ir, app_context_param);
                symtab_free(out_symtab, app_context_param);
                return ERR_SYNTAX;

            }


            uint32_t base = (state.section == SEC_TEXT) ? cfg->text_base : cfg->data_base;
            uint32_t pc = (state.section == SEC_TEXT) ? state.text_pc : state.data_pc;
            uint32_t addr = base + pc;


            e = symtab_add(out_symtab, statement.as.label.name, addr, app_context_param);

            if(e != ERR_OK){

                stmt_free_heap_parts(&statement);
                ir_free(out_ir, app_context_param);
                symtab_free(out_symtab, app_context_param);

                return e;

            }

        
            e = ir_push(out_ir, &statement, app_context_param);

            if(e != ERR_OK){

                stmt_free_heap_parts(&statement);
                ir_free(out_ir, app_context_param);
                symtab_free(out_symtab, app_context_param);

                return e;

            }
        }


        if(statement.kind == ST_EMPTY){

            continue;

        }

        if(statement.kind == ST_DIR_TEXT){

            state.section = SEC_TEXT;
            e = ir_push(out_ir, &statement, app_context_param);

            if(e != ERR_OK){

                stmt_free_heap_parts(&statement);
                ir_free(out_ir, app_context_param);
                symtab_free(out_symtab, app_context_param);

                return e;

            }

            continue;
        }

        if(statement.kind == ST_DIR_DATA){

            state.section = SEC_DATA;
            e = ir_push(out_ir, &statement, app_context_param);

            if(e != ERR_OK){

                stmt_free_heap_parts(&statement);
                ir_free(out_ir, app_context_param);
                symtab_free(out_symtab, app_context_param);

                return e;

            }

            continue;

        }


        if(statement.kind == ST_DIR_WORD){

            if(state.section != SEC_DATA){

                APP_ERROR(app_context_param, ".word directives are out of .data section");
                stmt_free_heap_parts(&statement);
                ir_free(out_ir, app_context_param);
                symtab_free(out_symtab, app_context_param);

                return ERR_SYNTAX;

            }

            state.data_pc = state.data_pc + (statement.as.dir_word.n * 4);  // a word is 4 byte and we have statement.as.dir.word.n words
            e = ir_push(out_ir, &statement, app_context_param);

            if(e != ERR_OK){

                stmt_free_heap_parts(&statement);
                ir_free(out_ir, app_context_param);
                symtab_free(out_symtab, app_context_param);

                return e;

            }

            continue;

        }


        if(statement.kind == ST_INSTR){

            if(state.section != SEC_TEXT){

                APP_ERROR(app_context_param, "Instruction outside of .text section");
                stmt_free_heap_parts(&statement);
                ir_free(out_ir, app_context_param);
                symtab_free(out_symtab, app_context_param);

                return ERR_SYNTAX;

            }

            state.text_pc += 4;
            
            e = ir_push(out_ir, &statement, app_context_param);

            if(e != ERR_OK){

                stmt_free_heap_parts(&statement);
                ir_free(out_ir, app_context_param);
                symtab_free(out_symtab, app_context_param);

                return e;

            }

            continue;

        }

        stmt_free_heap_parts(&statement);

    }

    *out_final_state = state;
    return ERR_OK;

}