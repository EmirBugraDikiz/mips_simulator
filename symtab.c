#include "symtab.h"
#include "error_handling.h"
#include <stdlib.h>
#include <string.h>

Err symtab_init(Symtab *st, app_context *app_context_param){

    if(!st){

        APP_ERROR(app_context_param, "INVALID ARGUMENT.");
        return ERR_INVALID_ARGUMENT;

    }

    st->v = NULL;
    st->n = st->cap = 0;

    return ERR_OK;

}


Err symtab_free(Symtab *st, app_context *app_context_param){

    if(!st) return ERR_INVALID_ARGUMENT;
    free(st->v);
    st->v = NULL;
    st->n = st->cap = 0;
    
    return ERR_OK;

}


int symtab_find(Symtab *st, const char *name){

    if(!st || !name) return -1;

    for(size_t i = 0; i < st->n; i++){

        if(strcmp(st->v[i].name, name) == 0) return (int)i;

    }

    return -1;

}

static Err symtab_grow(Symtab *st, app_context *app_context_param){

    size_t new_cap = (st->cap == 0)? 64 : st->cap * 2;
    
    Symbol *p = realloc(st->v, sizeof(*p) * new_cap);

    if(!p){

        APP_PERROR(app_context_param, "SYMBOL TABLE REALLOC FAILED.");
        return ERR_OOM;

    }

    st->v = p;
    st->cap = new_cap;

    return ERR_OK;

}

Err symtab_add(Symtab *st, const char *name, uint32_t addr, app_context *app_context_param){

    if(!st || !name) {

        APP_ERROR(app_context_param, "INVALID ARGUMENT");
        return ERR_INVALID_ARGUMENT;

    }

    if(symtab_find(st, name) >= 0){

        APP_ERROR(app_context_param, "DUPLICATE LABEL IN SYMBOL TABLE.");
        return ERR_SYNTAX;

    }

    if(st->n == st->cap){

        Err e = symtab_grow(st, app_context_param);
        if(e != ERR_OK) return e;

    }

    strncpy(st->v[st->n].name, name, sizeof(st->v[st->n].name) - 1);

    st->v[st->n].name[sizeof(st->v[st->n].name) - 1] = '\0';
    st->v[st->n].addr = addr;
    st->n++;

    return ERR_OK;

}

Err symtab_lookup(Symtab *st, const char *name, uint32_t *out_addr, app_context *app_context_param){

    if(!st || !name || !out_addr){

        APP_ERROR(app_context_param, "INVALID ARGUMENT.");
        return ERR_INVALID_ARGUMENT;

    }

    int index = symtab_find(st, name);

    if(index < 0) return ERR_UNDEF_LABEL;

    *out_addr = st->v[index].addr;

    return ERR_OK;

}
