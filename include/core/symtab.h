#ifndef SYMTAB_H
#define SYMTAB_H

#include <stdint.h>
#include <stdlib.h>
#include "error_handling.h"


typedef struct{

    char name[64];
    uint32_t addr;

}Symbol;

typedef struct{

    Symbol *v;
    size_t n;
    size_t cap;


}Symtab;


Err symtab_init(Symtab *st, app_context *app_context_param);
Err symtab_free(Symtab *st, app_context *app_context_param);
int symtab_find(Symtab *st, const char *name);
Err symtab_add(Symtab *st, const char *name, uint32_t addr, app_context *app_context_param);
Err symtab_lookup(Symtab *st, const char *name, uint32_t *out_addr, app_context *app_context_param);

#endif