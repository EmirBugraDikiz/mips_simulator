#ifndef SYMTAB_H
#define SYMTAB_H

#include <stdint.h>
#include <stdlib.h>
#include "error_handling.h"
#include "core/ir.h"

typedef struct{

    char name[64];
    uint32_t addr;
    Section section;

}Symbol;

typedef struct{

    Symbol *v;
    size_t n;
    size_t cap;


}Symtab;


Err symtab_init(Symtab *st, app_context *app_context_param);
Err symtab_free(Symtab *st, app_context *app_context_param);
int symtab_find(const Symtab *st, const char *name);
Err symtab_add(Symtab *st, const char *name, uint32_t addr, Section section, app_context *app_context_param);
Err symtab_lookup(const Symtab *st, const char *name, Symbol *out_sym, app_context *app_context_param);

#endif