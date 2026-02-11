#ifndef LINE_H
#define LINE_H
#include "error_handling.h"
#include <stdio.h>

typedef struct input_program_t input_program;

struct input_program_t{

    FILE* input;
    char **lines;
    size_t number_of_line;

}; 

Err read_line_fgets(app_context* app_context_param, FILE *f, char **out_line);

Err read_all_lines(app_context* app_context_param, FILE *f, char ***out_lines, size_t *out_n);

Err print_lines(app_context* app_context_param, char** lines, size_t n);

input_program* create_input_program(app_context* app_context_param, const char* input_file_path);

Err destroy_input_program(app_context* app_context_param, input_program* input_program_param);

#endif