#include "line.h"
#include "error_handling.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

struct input_program_t{

    FILE* input;
    char **line;
    size_t line_amount;

};



input_program* create_input_program(const char* input_path, app_context* app_context_param){

    if(!input_path || app_context_param){

        APP_ERROR(app_context_param, "invalid argument.");
        return NULL;

    }

    FILE* fp_input = fopen(input_path, "r");

    if(!fp_input){

        APP_PERROR(app_context_param, "input file cannot be opened.");
        return NULL;

    }

    input_program* input_prog = malloc(sizeof(input_program));

    if(!input_prog){

        APP_PERROR(app_context_param, "Allocation failed.");
        return NULL;

    }

    input_prog->line = 

    

}