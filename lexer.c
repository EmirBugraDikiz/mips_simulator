#include "lexer.h"
#include "error_handling.h"
#include <errno.h>
#include <ctype.h>
#include <string.h>



Err tokenvec_init(TokenVec *tv, app_context *app_context_param){

    if(!tv){

        APP_ERROR(app_context_param, "INVALID ARGUMENT.");
        return ERR_INVALID_ARGUMENT;

    }

    tv->v = NULL;
    tv->n = 0;
    tv->cap = 0;
    
    return ERR_OK;

}

void tokenvec_free(TokenVec *tv, app_context *app_context_param){

    if(!tv){

        APP_ERROR(app_context_param, "INVALID ARGUMENT.");
        return;

    } 

    free(tv->v);
    tv->v = NULL;
    tv->n = tv->cap = 0;

    return;
}

static Err tokenvec_grow(TokenVec *tv, app_context *app_context_param){

    size_t new_cap = (tv->cap == 0)? 64 : tv->cap * 2;

    Token *p = realloc(tv->v,sizeof(*p) * new_cap);

    if(!p){

        APP_PERROR(app_context_param, "TOKEN VECTOR REALLOC FAILED.");
        return ERR_OOM;

    }

    tv->v = p;
    tv->cap = new_cap;

    return ERR_OK;

}

static Err tokenvec_push(TokenVec *tv, const Token *t, app_context *app_context_param){

    if(tv->n == tv->cap){

        Err e = tokenvec_grow(tv, app_context_param);
        
        if(e != ERR_OK) return e;

    }

    tv->v[tv->n++] = *t;
     
    return ERR_OK;
}

static TokKind classify_word(const char *w){

    if(w[0] == '.') return TOK_DOT;
    
    if(w[0] == '$') return TOK_REG;

    char *end = NULL;
    errno = 0;

    strtol(w, &end, 0);

    if(end && *end == '\0' && errno == 0) return TOK_INT;

    return TOK_IDENT;

}


Err lex_line(const char *line, int line_no, TokenVec *out, app_context *app_context_param){

    Err e = tokenvec_init(out, app_context_param);
    if(e != ERR_OK) return e;

    size_t i = 0;

   while(line[i]){

        while(line[i] && isspace((unsigned char)line[i])) i++;
        
        if(!line[i]) break;

        char c = line[i];
        Token t = {0};
        t.line_no = line_no;
        t.column_no = (int)i + 1;

        // single tokens

        if(c == ':') {t.kind = TOK_COLON; strcpy(t.lexeme, ":"); i++; e = tokenvec_push(out, &t, app_context_param); if (e != ERR_OK) return e; continue;}
        if(c == ',') {t.kind = TOK_COMMA; strcpy(t.lexeme, ","); i++; e = tokenvec_push(out, &t, app_context_param); if(e != ERR_OK) return e; continue;}
        if(c == '(') {t.kind = TOK_LPAREN; strcpy(t.lexeme, "("); i++; e = tokenvec_push(out, &t, app_context_param); if(e != ERR_OK) return e; continue;}
        if(c == ')') {t.kind = TOK_RPAREN; strcpy(t.lexeme, ")"); i++; e = tokenvec_push(out, &t, app_context_param); if(e != ERR_OK) return e; continue;}

        // word token part

        char buf[64];
        size_t b = 0;
        size_t start = i;

        while(line[i]){

            char word_token_start = line[i];

            if(isspace((unsigned char)word_token_start)) break;
            
            if(word_token_start == ':' || word_token_start == ',' || word_token_start == '(' || word_token_start == ')') break;

            if(b + 1 < sizeof(buf)) buf[b++] = word_token_start;
            i++;

        }

        buf[b] = '\0';

        if(b == 0) continue;

        t.column_no = (int)start + 1;
        t.kind = classify_word(buf);
        strncpy(t.lexeme, buf, sizeof(t.lexeme) - 1);
        t.lexeme[sizeof(t.lexeme) - 1] = '\0';

        e = tokenvec_push(out, &t, app_context_param);
        if(e != ERR_OK) return e;

   }

   return ERR_OK;


}