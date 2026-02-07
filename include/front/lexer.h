#ifndef LEXER_H
#define LEXER_H


#include <stdlib.h>
#include "core/error_handling.h"


typedef enum{

    TOK_IDENT,
    TOK_DOT,
    TOK_REG,
    TOK_INT,
    TOK_COLON,
    TOK_COMMA,
    TOK_LPAREN,
    TOK_RPAREN

}TokKind;

typedef struct{

    TokKind kind;
    char lexeme[64];      // lexeme is the content that a token has
    size_t line_no;       
    size_t column_no;


}Token;


typedef struct{

    Token *v;
    size_t n;
    size_t cap;

}TokenVec;


Err tokenvec_init(TokenVec *tv, app_context *app_context_param);
void tokenvec_free(TokenVec *tv, app_context *app_context_param);

Err lex_line(const char *line, int line_no, TokenVec *out, app_context *app_context_param);

#endif