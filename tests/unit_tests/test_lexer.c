#include "test.h"
#include "front/lexer.h"
#include "core/error_handling.h"
#include <stdio.h>
#include <string.h>


typedef struct token_expression_t{

    TokKind kind;
    const char *lex;

}token_expression;


typedef struct{

    const char *name;
    const char* input;
    size_t expected_n;
    token_expression expected[16];

}LexCase;

typedef struct{

    const char *name;
    const LexCase *cases;
    size_t count;

}LexSuite;



static void run_lex_case(const char *suite_name, const LexCase *test_case, app_context *app_context_param){

    TokenVec tv = {0};
    
    (void)app_context_param;

    Err e = lex_line(test_case->input, 1, &tv, app_context_param);
    
    if(e != ERR_OK){

        fprintf(stderr, "\n[CASE] suite = %s case = %s input = \"%s\"\n",
        suite_name, test_case->name, test_case->input);

    }
    
    ASSERT_EQ_INT(e, ERR_OK);

    if(tv.n != test_case->expected_n){

        fprintf(stderr, "\n[CASE] suite = %s case = %s input = \"%s\"\n",
        suite_name, test_case->name, test_case->input);

    }

    ASSERT_EQ_INT((int)tv.n, (int)test_case->expected_n);

    for(size_t i = 0; i < test_case->expected_n; i++){

        ASSERT_EQ_INT((int)tv.v[i].kind, test_case->expected[i].kind);
        ASSERT_STREQ(tv.v[i].lexeme, test_case->expected[i].lex);
    }

    tokenvec_free(&tv, app_context_param);

}


static void run_lex_table(const LexSuite *suite, app_context *app_context_param){

    (void)app_context_param;

    for(size_t i = 0; i < suite->count; i++){

        run_lex_case(suite->name, &suite->cases[i], app_context_param);

    }

}


static const LexCase g_delimiter_cases[] = {
    { "delims_compact",
      ":,()",
      4,
      { {TOK_COLON,":"}, {TOK_COMMA,","}, {TOK_LPAREN,"("}, {TOK_RPAREN,")"} } },

    { "delims_spaced",
      "  (  ,  )  :  ",
      4,
      { {TOK_LPAREN,"("}, {TOK_COMMA,","}, {TOK_RPAREN,")"}, {TOK_COLON,":"} } },

    { "label_delim_no_space",
      "loop:add",
      3,
      { {TOK_IDENT,"loop"}, {TOK_COLON,":"}, {TOK_IDENT,"add"} } },

    { "paren_pair",
      "( )",
      2,
      { {TOK_LPAREN,"("}, {TOK_RPAREN,")"} } },

    { "comma_chain",
      ",,,",
      3,
      { {TOK_COMMA,","}, {TOK_COMMA,","}, {TOK_COMMA,","} } },
};


static const LexCase g_ident_and_directive_cases[] = {
    { "ident_add", "add", 1, { {TOK_IDENT,"add"} } },
    { "ident_main", "main", 1, { {TOK_IDENT,"main"} } },
    { "ident_underscore", "_start", 1, { {TOK_IDENT,"_start"} } },
    { "ident_alnum", "loop1", 1, { {TOK_IDENT,"loop1"} } },

    // allowed . token in an identifier but not allowed yet at the beginning 
    { "ident_with_dot", "L.blablah", 1, { {TOK_IDENT,"L.blablah"} } },

    { "dir_text", ".text", 1, { {TOK_DOT,".text"} } },
    { "dir_data", ".data", 1, { {TOK_DOT,".data"} } },
    { "dir_word", ".word", 1, { {TOK_DOT,".word"} } },

    // unknown directive. Lexer will lex it but parser will reject it (.globl is not supported).
    { "dir_unknown", ".globl", 1, { {TOK_DOT,".globl"} } },
};



static const LexCase g_register_cases[] = {
    { "reg_t0", "$t0", 1, { {TOK_REG,"$t0"} } },
    { "reg_sp", "$sp", 1, { {TOK_REG,"$sp"} } },
    { "reg_zero_num", "$0", 1, { {TOK_REG,"$0"} } },
    { "reg_31_num", "$31", 1, { {TOK_REG,"$31"} } },

    // invalid register. Lexer will lex it but parser will reject it.
    { "reg_bad", "$bad", 1, { {TOK_REG,"$bad"} } },

    { "regs_many", "$t0 $t1 $t2", 3,
      { {TOK_REG,"$t0"}, {TOK_REG,"$t1"}, {TOK_REG,"$t2"} } },
};



static const LexCase g_int_cases[] = {
    { "int_zero", "0", 1, { {TOK_INT,"0"} } },
    { "int_pos", "123", 1, { {TOK_INT,"123"} } },
    { "int_neg", "-4", 1, { {TOK_INT,"-4"} } },

    { "int_hex", "0x10", 1, { {TOK_INT,"0x10"} } },
    { "int_hex_neg", "-0x10", 1, { {TOK_INT,"-0x10"} } },

    { "int_oct", "077", 1, { {TOK_INT,"077"} } },

    // lexer tokenize to int but parser can be detect overflow in another test.
    { "int_big", "2147483647", 1, { {TOK_INT,"2147483647"} } },

    { "ints_many", "1 -2 0x3 077", 4,
      { {TOK_INT,"1"}, {TOK_INT,"-2"}, {TOK_INT,"0x3"}, {TOK_INT,"077"} } },
};



static const LexCase g_real_line_cases[] = {
    { "lw_compact",
      "lw $t0,4($sp)",
      7,
      { {TOK_IDENT,"lw"},
        {TOK_REG,"$t0"},
        {TOK_COMMA,","},
        {TOK_INT,"4"},
        {TOK_LPAREN,"("},
        {TOK_REG,"$sp"},
        {TOK_RPAREN,")"} } },

    { "lw_spaced",
      "lw   $t0 ,  4 ( $sp )",
      7,
      { {TOK_IDENT,"lw"},
        {TOK_REG,"$t0"},
        {TOK_COMMA,","},
        {TOK_INT,"4"},
        {TOK_LPAREN,"("},
        {TOK_REG,"$sp"},
        {TOK_RPAREN,")"} } },

    { "add_regs",
      "add $t0, $t1, $t2",
      6,
      { {TOK_IDENT,"add"},
        {TOK_REG,"$t0"}, {TOK_COMMA,","},
        {TOK_REG,"$t1"}, {TOK_COMMA,","},
        {TOK_REG,"$t2"} } },

    { "beq_label",
      "beq $t0, $t1, loop",
      6,
      { {TOK_IDENT,"beq"},
        {TOK_REG,"$t0"}, {TOK_COMMA,","},
        {TOK_REG,"$t1"}, {TOK_COMMA,","},
        {TOK_IDENT,"loop"} } },

    { "label_and_instr",
      "loop: add $t0,$t1,$t2",
      8,
      { {TOK_IDENT,"loop"}, {TOK_COLON,":"},
        {TOK_IDENT,"add"},
        {TOK_REG,"$t0"}, {TOK_COMMA,","},
        {TOK_REG,"$t1"}, {TOK_COMMA,","},
        {TOK_REG,"$t2"} } },

    { "word_line",
      ".word -1, 0x10, 077",
      6,
      { {TOK_DOT,".word"},
        {TOK_INT,"-1"}, {TOK_COMMA,","},
        {TOK_INT,"0x10"}, {TOK_COMMA,","},
        {TOK_INT,"077"} } },

    {"label_then_instruction_add",
        "main: add $t0, $t1, $t2",
        8,
        {{TOK_IDENT, "main"}, {TOK_COLON, ":"},
          {TOK_IDENT, "add"}, {TOK_REG, "$t0"},
          {TOK_COMMA, ","},   {TOK_REG, "$t1"}, 
          {TOK_COMMA, ","}, {TOK_REG, "$t2"}}}
};



static const LexSuite g_lex_suites[] = {

    {"delimiters", g_delimiter_cases, ARR_LEN(g_delimiter_cases)},
    {"identifier+directive", g_ident_and_directive_cases, ARR_LEN(g_ident_and_directive_cases)},
    {"registers", g_register_cases, ARR_LEN(g_register_cases)},
    {"integers", g_int_cases, ARR_LEN(g_int_cases)},
    {"real-lines", g_real_line_cases, ARR_LEN(g_real_line_cases)}

};



void test_lex_all_tables(app_context *app_context_param){

    (void)app_context_param;

    for(size_t i = 0; i < ARR_LEN(g_lex_suites); i++){

        run_lex_table(&g_lex_suites[i], app_context_param);

    }

}
