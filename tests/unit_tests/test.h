#ifndef TEST_H
#define TEST_H

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../lexer.h"


#define ARR_LEN(a) (sizeof(a)/sizeof((a)[0]))


// INLINE TEST HELPER

static inline void test_fail(const char *file, int line, const char *expression, const char *message){

    fprintf(stderr, "\n[TEST FAIL]\n"
                    " Location : %s:%d:\n"
                    " Expr     : %s\n"
                    " Message  : %s\n\n",
                    file, line,
                    expression? expression : "(none)",
                    message ? message : "(none)");
    
    exit(EXIT_FAILURE);

}

// ASSERT MACROS HELPER

#define ASSERT_EQ_INT(a, b) do{\
    long long _a = (long long)(a);\
    long long _b = (long long)(b);\
    if(_a != _b){\
        char _buf[256];\
        snprintf(_buf, sizeof(_buf),\
                 "Expected %s == %s, but got %lld vs %lld",\
                 #a, #b, _a, _b);\
        test_fail(__FILE__, __LINE__, #a "==" #b, _buf);\
    }\
}while(0)    


#define ASSERT_STREQ(a, b) do{\
    const char *_a = (a);\
    const char *_b = (b);\
    if((_a == NULL && _b != NULL) || (_a != NULL && _b == NULL) || (_a && _b && strcmp(_a, _b) != 0)){\
        char _buf[256];\
        snprintf(_buf, sizeof(_buf),\
                 "Expected strings equal:\n"\
                 " %s = \"%s\"\n"\
                 " %s = \"%s\"",\
                 #a, _a ? _a : "(null)",\
                 #b, _b ? _b : "(null)");\
        test_fail(__FILE__, __LINE__, "strcmp(" #a ", " #b ")", _buf);\
    }\
}while(0)
        
void test_lex_all_tables(app_context *app_context_param);

void test_preprocess_all_tables();

void test_parser_tables(app_context *app_context_param);

#endif