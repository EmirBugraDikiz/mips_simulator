#include "test.h"
#include "../../ir.h"
#include "stdint.h"
#include <stdio.h>
#include <string.h>
#include "../../parser.h"
#include "../../preprocess.h"

typedef struct{

    const char *name;
    const char *input;

    Err expected_error;
    StatementKind expected_kind;

    union{

        const char* st_label_name;
        struct{

            int32_t values[8];
            size_t n;

        }st_dir_word;

        struct{

            char mnemonic[16];
            Operand ops[3];
            int operand_count;

        }st_instruction;

    }st;

}ParseCase;


static void assert_stmt_matches(const ParseCase *test_case, Statement *statement){

    ASSERT_EQ_INT((int)statement->kind, (int)test_case->expected_kind);
    
    if(test_case->expected_kind == ST_LABEL){

        ASSERT_STREQ(statement->as.label.name, test_case->st.st_label_name);

    }

    if(test_case->expected_kind == ST_INSTR){

        ASSERT_STREQ(statement->as.instr.mnemonic, test_case->st.st_instruction.mnemonic);
        ASSERT_EQ_INT((int)statement->as.instr.op_count, (int)test_case->st.st_instruction.operand_count);


        for(size_t i = 0; i < statement->as.instr.op_count; i++){

            ASSERT_EQ_INT((int)statement->as.instr.ops[i].kind, (int)test_case->st.st_instruction.ops[i].kind);
            if(test_case->st.st_instruction.ops[i].kind == OP_REGISTER){

                ASSERT_EQ_INT((int)statement->as.instr.ops[i].v.reg, (int)test_case->st.st_instruction.ops[i].v.reg);

            }   
            
            if(test_case->st.st_instruction.ops[i].kind == OP_IMMEDIATE){

                ASSERT_EQ_INT((int)statement->as.instr.ops[i].v.imm, (int)test_case->st.st_instruction.ops[i].v.imm);

            }


            if(test_case->st.st_instruction.ops[i].kind ==  OP_LABEL){

                ASSERT_STREQ(statement->as.instr.ops[i].v.label, test_case->st.st_instruction.ops[i].v.label);

            }

            if(test_case->st.st_instruction.ops[i].kind == OP_MEMORY){

                ASSERT_EQ_INT((int)statement->as.instr.ops[i].v.mem.offset, (int)test_case->st.st_instruction.ops[i].v.mem.offset);
                ASSERT_EQ_INT((int)statement->as.instr.ops[i].v.mem.base_reg, (int)test_case->st.st_instruction.ops[i].v.mem.base_reg);

            }

        }


    }


    if(test_case->expected_kind == ST_DIR_WORD){

        ASSERT_EQ_INT((int)statement->as.dir_word.n, test_case->st.st_dir_word.n);

        for(size_t i = 0; i < test_case->st.st_dir_word.n; i++){

            ASSERT_EQ_INT((int)statement->as.dir_word.values[i], test_case->st.st_dir_word.values[i]);

        }

    }

}


static void run_parse_case(const ParseCase *test_case, app_context *app_context_param){

    char temp_buf[1024];

    strncpy(temp_buf, test_case->input, sizeof(temp_buf) - 1);
    temp_buf[sizeof(temp_buf) - 1] = '\0';

    strip_comment(temp_buf);
    trim_inplace(temp_buf);

    TokenVec tv;
    Err e = lex_line(temp_buf, 1, &tv, app_context_param);

    if(e != ERR_OK){

        fprintf(stderr, "\n[PARSE CASE] %s\n  input=\"%s\"\n", test_case->name, test_case->input);

    }

    ASSERT_EQ_INT(e, ERR_OK);

    Statement s;
    memset(&s, 0, sizeof(s));

    int has_label = 0;
    Err pe = parse_line(app_context_param, &tv, temp_buf, 1, &has_label, &s);

    if(pe != test_case->expected_error){

        fprintf(stderr, "\n[PARSE CASE] %s\n  input=\"%s\"\n", test_case->name, test_case->input);

    }


    ASSERT_EQ_INT(pe, test_case->expected_error);

    if(pe == ERR_OK){

        assert_stmt_matches(test_case, &s);
        stmt_free_heap_parts(&s);

    }

    tokenvec_free(&tv, app_context_param);

}

static void run_parse_table(const ParseCase *cases, size_t n, app_context *app_context_param){

    for(size_t i = 0; i < n; i++){

        run_parse_case(&cases[i], app_context_param);

    }

}



static const ParseCase g_parser_ok_cases[] = {

    {"directive_text",
        ".text",
        ERR_OK,
        ST_DIR_TEXT,
    {0}},

    {"directive_data",
        ".data",
        ERR_OK,
        ST_DIR_DATA,
    {0}},

    {"directive_word_3",
        ".word 10, 20, -1",
        ERR_OK,
        ST_DIR_WORD,
        {.st_dir_word = {{10, 20, -1}, 3}}},

    {"label_only",
        "main:",
        ERR_OK,
        ST_LABEL,
        {.st_label_name = "main"}},

    {"instruction_add",
        "add $t0, $t1, $t2",
        ERR_OK,
        ST_INSTR,
        {.st_instruction = {"add",
        {{.kind = OP_REGISTER, .v.reg = 8}, {.kind = OP_REGISTER, .v.reg = 9}, {.kind = OP_REGISTER, .v.reg = 10}},
        3}}},

    {"instruction_lw",
        "lw $t0, 4($sp)",
        ERR_OK,
        ST_INSTR,
        {.st_instruction = {"lw", {{.kind = OP_REGISTER, .v.reg = 8}, {.kind = OP_MEMORY, .v.mem = {4, 29}}},
        2}}},

    

};



static const ParseCase g_parser_bad_cases[] = {

    {"word_missing_int",
        ".word",
        ERR_SYNTAX,
        -1,
        {0}},

    {"label_double_colon",
        "main::",
        ERR_SYNTAX,
        -1,
        {0}},

    {"instruction_missing_operand",
        "add $t0, $t1,",
        ERR_SYNTAX,
        -1,
        {0}}

};

void test_parser_tables(app_context *app_context_param){

    run_parse_table(g_parser_ok_cases, ARR_LEN(g_parser_ok_cases), app_context_param);
    run_parse_table(g_parser_bad_cases, ARR_LEN(g_parser_bad_cases), app_context_param);

}
