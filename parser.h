#ifndef PARSER_H
#define PARSER_H

#include "ir.h"
#include "lexer.h"

Err parse_line(app_context *app_context_param, const TokenVec *tv, const char *source_line, int line_no, char out_label[64], int *out_has_label, Statement *out_statement);

#endif