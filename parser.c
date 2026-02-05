#include "parser.h"
#include "error_handling.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include "lexer.h"
#include "ir.h"
#include "regmap.h"
#include "isa_mips.h"



static void report_syntax(app_context *app_context_param, int line_no, int column_no, const char *message, const char *source_line){

    (void)app_context_param;

    fprintf(stderr, "Syntax error at line %d, col %d: %s\n", line_no, column_no, message);

    if(source_line){

        fprintf(stderr, " %s\n", source_line);
        fprintf(stderr, " ");

        for(int i = 0; i < column_no; i++) fputc(' ', stderr);
        fprintf(stderr, "^\n");

    }

}


static int parse_reg_name(const char *lex){

    if(!lex || lex[0] != '$') return -1;

    if(isdigit((unsigned char)lex[1])){

        char *end = NULL;
        long v = strtol(lex + 1, &end, 10);
        if(end && *end == '\0' && v >= 0 && v <= 31) return (int)v;
        return -1;

    }

    return regmap_lookup(lex);

}

static Err parse_int32(const char *lex, int32_t *out){

    if(!lex || !out) return ERR_INVALID_ARGUMENT;

    errno = 0;
    char *end = NULL;
    long v = strtol(lex, &end, 0);

    if(!end || *end != '\0' || errno != 0) return ERR_SYNTAX;
    if(v < INT32_MIN || v > INT32_MAX) return ERR_SYNTAX;     // overflow

    *out = (int32_t)v;

    return ERR_OK;
}


static char *dup_cstr(app_context *app_context_param, const char *s){

    char *p = strdup(s);
    if(!p) APP_PERROR(app_context_param, "STRDUP FAILED.");
    return p;

}


static Err parse_operand(app_context *app_context_param, const TokenVec *tv, size_t *pos, Operand *out_operand, const char *source_line){

    if(!tv || !pos || !out_operand) return ERR_INVALID_ARGUMENT;

    if(*pos >= tv->n) return ERR_SYNTAX;

    const Token *t = &tv->v[*pos];

    // MEM: INT (REG)

    if(t->kind == TOK_INT){

        // lookahead: INT LPAREN REG RPAREN
        if(*pos + 3 < tv->n && tv->v[*pos + 1].kind == TOK_LPAREN && tv->v[*pos + 2].kind == TOK_REG && tv->v[*pos + 3].kind == TOK_RPAREN){

            int32_t off = 0;
            Err e = parse_int32(t->lexeme, &off);
            
            if(e != ERR_OK){

                report_syntax(app_context_param, t->line_no, t->column_no, "INVALID OFFSET IMMEDIATE", source_line);
                return ERR_SYNTAX;

            }

            int base = parse_reg_name(tv->v[*pos + 2].lexeme);

            if(base < 0) {

                report_syntax(app_context_param, tv->v[*pos + 2].line_no, tv->v[*pos + 2].column_no, "INVALID BASE REGISTER", source_line);
                return ERR_SYNTAX;

            }

            out_operand->kind = OP_MEMORY;
            out_operand->v.mem.offset = off;
            out_operand->v.mem.base_reg = base;
            *pos += 4;
            
            return ERR_OK;

        }

        //otherwise this is an immediate

        int32_t imm = 0;
        Err e = parse_int32(t->lexeme, &imm);

        if(e != ERR_OK){

            report_syntax(app_context_param, t->line_no, t->column_no, "INVALID IMMEDIATE", source_line);
            return ERR_SYNTAX;

        }

        out_operand->kind = OP_IMMEDIATE;
        out_operand->v.imm = imm;
        (*pos)++;

        return ERR_OK;

    }


    if(t->kind == TOK_REG){

        int r = parse_reg_name(t->lexeme);

        if(r < 0){

            report_syntax(app_context_param, t->line_no, t->column_no, "INVALID REGISTER NAME", source_line);
            return ERR_SYNTAX;

        }

        out_operand->kind = OP_REGISTER;
        out_operand->v.reg = r;
        (*pos)++;

        return ERR_OK;


    }

    if(t->kind == TOK_IDENT){

        // label reference

        char *name = dup_cstr(app_context_param, t->lexeme);
        
        if(!name) return ERR_OOM;

        out_operand->kind = OP_LABEL;
        out_operand->v.label = name;
        (*pos)++;

        return ERR_OK;
    }

    report_syntax(app_context_param, t->line_no, t->column_no, "UNEXPECTED TOKEN IN OPERAND", source_line);
    
    return ERR_SYNTAX;


}

static int tok_is_dot(const Token *t, const char *s){

    return t && t->kind == TOK_DOT && strcmp(t->lexeme, s) == 0;

}

Err parse_line(app_context *app_context_param, const TokenVec *tv, const char *source_line, int line_no, int *out_has_label, Statement *out_statement){

    if(!tv || !out_statement || !out_has_label) return ERR_INVALID_ARGUMENT;

    *out_has_label = 0;

    memset(out_statement, 0, sizeof(*out_statement));
    out_statement->line_no = line_no;

    size_t pos = 0;

    int has_label_prefix = 0;
    char label_name[64] = {0};

    if(tv->n >= 2 && tv->v[0].kind == TOK_IDENT && tv->v[1].kind == TOK_COLON){

        has_label_prefix = 1;
        strncpy(label_name, tv->v[0].lexeme, sizeof(label_name) - 1);
        label_name[sizeof(label_name) - 1] = '\0';
        pos = 2;

        if(pos < tv->n && tv->v[pos].kind == TOK_COLON){

            report_syntax(app_context_param, line_no, (int)tv->v[pos].column_no, "double colon is invalid.", source_line);
            return ERR_SYNTAX;

        }


        if(pos >= tv->n){

            *out_has_label = 1;
            out_statement->kind = ST_LABEL;
            strncpy(out_statement->as.label.name, label_name, sizeof(out_statement->as.label.name) - 1);
            out_statement->as.label.name[sizeof(out_statement->as.label.name) - 1] = '\0';
            return ERR_OK;

        }

        if(tv->v[pos].kind != TOK_IDENT && tv->v[pos].kind != TOK_DOT){

            report_syntax(app_context_param, line_no, (int)tv->v[pos].column_no, "expected instruction or .word after label.", source_line);
            return ERR_SYNTAX;
        }


    }

    // directive?

    if(pos < tv->n && tv->v[pos].kind == TOK_DOT){

        if(tok_is_dot(&tv->v[pos], ".text")){

            if(has_label_prefix){

                report_syntax(app_context_param, line_no, (int)tv->v[pos].column_no, "label prefix allowed only for instruction or .word", source_line);
                return ERR_SYNTAX;

            }

            out_statement->kind = ST_DIR_TEXT;
            return ERR_OK;

        }

        if(tok_is_dot(&tv->v[pos], ".data")){

            if(has_label_prefix){

                report_syntax(app_context_param, line_no, (int)tv->v[pos].column_no, "label prefix allowed only for instruction or .word", source_line);
                return ERR_SYNTAX;

            }

            out_statement->kind = ST_DIR_DATA;
            return ERR_OK;

        }

        if(tok_is_dot(&tv->v[pos], ".word")){

            pos++;

            if(pos >= tv->n || tv->v[pos].kind != TOK_INT){

                report_syntax(app_context_param, line_no, (pos < tv->n ? tv->v[pos].column_no : 1), ".word expects at least one integer", source_line);
                return ERR_SYNTAX;

            }

            size_t cap = 8;
            size_t n = 0;

            int32_t *values = malloc(sizeof(*values) * cap);

            if(!values){

                APP_PERROR(app_context_param, "MALLOC FAILED");
                return ERR_OOM;

            }

            while(pos < tv->n){

                if(tv->v[pos].kind != TOK_INT){

                    report_syntax(app_context_param, tv->v[pos].line_no, tv->v[pos].column_no, ".word expects an integer", source_line);
                    free(values);
                    return ERR_SYNTAX;

                }


                int32_t value = 0;
                if(parse_int32(tv->v[pos].lexeme, &value) != ERR_OK){

                    report_syntax(app_context_param, tv->v[pos].line_no, tv->v[pos].column_no, "invalid .word integer", source_line);
                    free(values);
                    return ERR_SYNTAX;

                }

                if(n == cap){

                    cap*=2;
                    int32_t *p = realloc(values, sizeof(*p) * cap);

                    if(!p){

                        APP_PERROR(app_context_param, "REALLOC FAILED");
                        free(values);
                        return ERR_OOM;

                    }

                    values = p;
                }

                values[n++] = value;
                pos++;


                if(pos < tv->n){

                    if(tv->v[pos].kind != TOK_COMMA){

                        report_syntax(app_context_param, tv->v[pos].line_no, tv->v[pos].column_no, "expected ',' between .word values", source_line);
                        free(values);
                        return ERR_SYNTAX;

                    }

                    pos++;

                    if(pos >= tv->n){

                        report_syntax(app_context_param, line_no, tv->v[pos - 1].column_no, "trailing comma in .word", source_line);
                        free(values);
                        return ERR_SYNTAX;

                    }

                }

                

            }

            if(has_label_prefix){

                out_statement->kind = ST_LABEL_PLUS_DIR_WORD;
                strncpy(out_statement->as.label_plus_dir_word.name, label_name, sizeof(out_statement->as.label_plus_dir_word.name) - 1);
                out_statement->as.label_plus_dir_word.name[sizeof(out_statement->as.label_plus_dir_word.name) - 1] = '\0';
                out_statement->as.label_plus_dir_word.dir_word.values = values;
                out_statement->as.label_plus_dir_word.dir_word.n = n;

                return ERR_OK;


            }

            out_statement->kind = ST_DIR_WORD;
            out_statement->as.dir_word.values = values;
            out_statement->as.dir_word.n = n;
            return ERR_OK;

        }

        report_syntax(app_context_param, tv->v[pos].line_no, tv->v[pos].column_no, "unknown directive", source_line);
        return ERR_SYNTAX;

    }


    // instruction part : IDENT operand || IDENT operand1, operand2 and so on.


    if(pos < tv->n && tv->v[pos].kind == TOK_IDENT){

      
        char mnemonic[16] = {0};
        Operand ops[3] = {0};
        int operand_count = 0;

        strncpy(mnemonic, tv->v[pos].lexeme, sizeof(mnemonic) - 1);
        mnemonic[sizeof(mnemonic) - 1] = '\0';
        int mnemonic_col = (int)tv->v[pos].column_no;
        pos++;

        while(pos < tv->n){

            if(tv->v[pos].kind == TOK_COMMA){

                pos++;
                continue;

            }

            if(operand_count >= 3){

                report_syntax(app_context_param, (int)tv->v[pos].line_no, (int)tv->v[pos].column_no, "too many operands (max 3).", source_line);
                for(size_t i = 0; i < operand_count; i++){

                    operand_free(&ops[i]);

                }
                return ERR_SYNTAX;

            }


            Operand operand = {0};
            Err e = parse_operand(app_context_param, tv, &pos, &operand, source_line);
            if(e != ERR_OK){

                for(size_t i = 0; i < operand_count; i++){

                    operand_free(&ops[i]);

                }

                return e;
            }

            ops[operand_count++] = operand;
        }

        const InstructionSpec *instruction_spec = isa_lookup(mnemonic);

        if(!instruction_spec || instruction_spec->op_count != operand_count || !are_operands_valid(instruction_spec, ops, operand_count)){

            report_syntax(app_context_param, line_no, mnemonic_col, "May be invalid mnemonic, wrong operand count or operand types mismatch.", source_line);
            for(size_t i = 0; i < operand_count; i++){

                operand_free(&ops[i]);

            }

            return ERR_SYNTAX;
        }


        if(has_label_prefix){

            out_statement->kind = ST_LABEL_PLUS_INSTR;

            strncpy(out_statement->as.label_plus_instr.name, label_name, sizeof(out_statement->as.label_plus_instr.name) - 1);
            out_statement->as.label_plus_instr.name[sizeof(out_statement->as.label_plus_instr.name) - 1] = '\0';

            strncpy(out_statement->as.label_plus_instr.instr.mnemonic, mnemonic, sizeof(out_statement->as.label_plus_instr.instr.mnemonic) - 1);
            out_statement->as.label_plus_instr.instr.mnemonic[sizeof(out_statement->as.label_plus_instr.instr.mnemonic) - 1] = '\0';

            out_statement->as.label_plus_instr.instr.op_count = operand_count;

            for(size_t i = 0; i < operand_count; i++){

                out_statement->as.label_plus_instr.instr.ops[i] = ops[i];

            }

            *out_has_label = 1;
            
            return ERR_OK;

        }

        else{

            out_statement->kind = ST_INSTR;

            strncpy(out_statement->as.instr.mnemonic, mnemonic, sizeof(out_statement->as.instr.mnemonic) - 1);
            out_statement->as.instr.mnemonic[sizeof(out_statement->as.instr.mnemonic) - 1] = '\0';

            out_statement->as.instr.op_count = operand_count;

            for(size_t i = 0; i < operand_count; i++){

                out_statement->as.instr.ops[i] = ops[i];

            }

            return ERR_OK;
            
        }
        

    }

    int col = (pos < tv->n) ? (int)tv->v[pos].column_no : (tv->n ? (int)tv->v[tv->n - 1].column_no : 1);

    report_syntax(app_context_param, line_no, col, "unrecognized statement", source_line);
    return ERR_SYNTAX;

}