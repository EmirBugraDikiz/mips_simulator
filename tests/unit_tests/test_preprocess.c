#include "test.h"
#include <stdio.h>
#include <string.h>
#include "front/preprocess.h"


typedef struct{

    const char *name;
    const char *input;
    const char *expected_line;

}PreprocessCase;


typedef struct{

    const char *name;
    const PreprocessCase *cases;
    size_t count;

}PreprocessSuite;


static void run_preprocess_case(const char *suite_name, const PreprocessCase *test_case){

    char temp_buf[1024];
    
    strncpy(temp_buf, test_case->input, sizeof(temp_buf) - 1);
    temp_buf[sizeof(temp_buf) - 1] = '\0';

    strip_comment(temp_buf);
    trim_inplace(temp_buf);

    if(strcmp(temp_buf, test_case->expected_line) != 0){

        fprintf(stderr, "\n[CASE] suite = %s case = %s\n"
                                       " input     = \"%s\"\n"
                                       " got       = \"%s\"\n"
                                       " expected  = \"%s\"\n",
                                    suite_name, test_case->name, test_case->input, temp_buf, test_case->expected_line);
        
        test_fail(__FILE__, __LINE__, "strcmp(preprocessed, expected)", "preprocess mismatch");


    }

}


static void run_preprocess_table(const PreprocessSuite *suite){

    for(size_t i = 0; i < suite->count; i++){

        run_preprocess_case(suite->name, &suite->cases[i]);

    }

}




static const PreprocessCase g_preprocess_comment_cases[] = {

    { "empty", "", "" },
    { "spaces_only", "   \t  ", "" },
    { "comment_only", "# only comment", "" },
    { "comment_only_leading_spaces", "   # comment after spaces", "" },
    { "comment_hash_only", "#", "" },
    { "comment_no_space_after_hash", "#comment", "" },
    { "comment_spaces_after_hash", "#      ", "" },
    { "code_then_comment_spaces", "add $t0, $t1  # trailing", "add $t0, $t1" },
    { "code_then_comment_no_space", "add $t0,$t1#x", "add $t0,$t1" },
    { "label_then_comment", "main:  # entry", "main:" },
    { "dir_then_comment", ".data  # data section", ".data" },
    { "word_then_comment", ".word 10, 20, 30 # init", ".word 10, 20, 30" },
    { "whitespace_then_comment_only", "      # nothing else", "" },
    { "comment_contains_delims", "# ) , : ( should be ignored", "" },
    { "comment_contains_directive", "# .word 1,2,3 just text", "" },
    {"label_instruction_then_comment", "main: add $t0, $t1, $t2   #instruction after label in a line.", "main: add $t0, $t1, $t2"},
    {"label_word_directive_then_comment", "array1: .word 10, 20, 30  # .word directive after label in a line.", "array1: .word 10, 20, 30"}

};


static const PreprocessCase g_preprocess_trim_cases[] = {

    { "trim_leading", "   add", "add" },
    { "trim_trailing", "add   ", "add" },
    { "trim_both", "   add   ", "add" },
    { "trim_tabs_newlines", "\t add \t", "add" },

    // preserve internal whitespaces
    { "trim_preserve_internal", "add   $t0,   $t1", "add   $t0,   $t1" },

};

// The lines that don't contain comments (including hash (#)) don't have to change.
static const PreprocessCase g_preprocess_no_comment_cases[] = {

    { "no_comment_simple", "add $t0, $t1", "add $t0, $t1" },
    { "no_comment_label", "loop:", "loop:" },
    { "no_comment_dir", ".text", ".text" },
    { "no_comment_parens", "lw $t0, 4($sp)", "lw $t0, 4($sp)" }

};


static const PreprocessSuite g_preprocess_suites[] = {

    { "comments",   g_preprocess_comment_cases,   ARR_LEN(g_preprocess_comment_cases) },
    { "trim",       g_preprocess_trim_cases,      ARR_LEN(g_preprocess_trim_cases) },
    { "no-comment", g_preprocess_no_comment_cases,ARR_LEN(g_preprocess_no_comment_cases) },

};


void test_preprocess_all_tables(){

    for(size_t i = 0; i < ARR_LEN(g_preprocess_suites); i++){

        run_preprocess_table(&g_preprocess_suites[i]);

    }

}