#include "line.h"
#include "error_handling.h"
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

struct input_program_t{

    FILE* input;
    char **lines;
    size_t number_of_line;

};



static Err append_chunk(app_context* app_context_param, char **destination, size_t *len, size_t *cap, const char *source, size_t n){

    if(!destination || !len || !cap || !source) {


        APP_ERROR(app_context_param, "INVALID ARGUMENT");
        return ERR_INVALID_ARGUMENT;

    }


    if(*cap < *len + n + 1){

        size_t new_cap = (*cap == 0)? 128 : *cap;
        while(new_cap < *len + n + 1) new_cap *= 2;


        char *p = realloc(*destination, new_cap);
        
        if(!p){

            APP_PERROR(app_context_param, "REALLOC FAILED");
            return ERR_OOM;

        } 
        

        *destination = p;
        *cap = new_cap;
    }


    memcpy(*destination + *len, source, n);
    *len += n;
    (*destination)[*len] = '\0';
    
    return ERR_OK;

}

Err read_line_fgets(app_context* app_context_param ,FILE *f, char** out_line){

    if(!f || !out_line) {

        APP_ERROR(app_context_param, "INVALID ARGUMENT");
        return ERR_INVALID_ARGUMENT;

    }

    size_t len = 0;
    size_t cap = 0;

    char chunk[256];

    while(fgets(chunk, sizeof(chunk), f)){

        Err e;

        size_t n = strlen(chunk);
        

        if((e = append_chunk(app_context_param ,out_line, &len, &cap, chunk, n)) != ERR_OK){

            free(*out_line);
            *out_line = NULL;
            return e;

        }


        if(n > 0 && chunk[n - 1] == '\n') return ERR_OK;


    }

    if(ferror(f)){

        free(*out_line);
        *out_line = NULL;
        APP_ERROR(app_context_param, "READ ERROR MAY BE OCCURED");
        return ERR_READ_ERROR;

    }

    if(len > 0) return ERR_OK;


    APP_ERROR(app_context_param, "END OF FILE REACHED & NO CHARACTER INPUT FOUND");
    return ERR_NO_CHAR_INPUT;

}

static Err push_line(app_context* app_context_param, char ***lines, size_t *n, size_t *cap, char *line){

    if(!lines || !n || !cap || !line){

        APP_ERROR(app_context_param, "INVALID ARGUMENT");
        return ERR_INVALID_ARGUMENT;

    }

    if(*cap == *n){

        size_t new_cap = (*cap == 0)? 64 : (*cap * 2);
        char **p = realloc(*lines, new_cap * sizeof(*p));
        
        if(!p){

            APP_PERROR(app_context_param, "REALLOC FAILED");
            return ERR_OOM;

        }

        *lines = p;
        *cap = new_cap;

    }


    (*lines)[(*n)++] = line;
    return ERR_OK;

}

Err read_all_lines(app_context* app_context_param, FILE *f, char ***out_lines, size_t *out_n){

    if(!app_context_param || !f || !out_lines || !out_n){

        APP_ERROR(app_context_param, "INVALID ARGUMENT");
        return ERR_INVALID_ARGUMENT;
    }

    *out_lines = NULL;
    *out_n = 0;

    char **lines = NULL;
    size_t n = 0; 
    size_t cap = 0;


    for(;;){

        char *line = NULL;
        Err e1 = read_line_fgets(app_context_param ,f, &line);

        if(e1 == ERR_NO_CHAR_INPUT) break;  
        if(e1 == ERR_READ_ERROR || e1 == ERR_OOM || e1 == ERR_INVALID_ARGUMENT){

            for(size_t i = 0; i < n; i++) free(lines[i]);
            free(lines);
            return e1;

        }

        Err e2;

        e2 = push_line(app_context_param, &lines, &n, &cap, line);

        if(e2 != ERR_OK){

            free(line);
            for(size_t i = 0; i < n; i++) free(lines[i]);
            free(lines);
            return e2;

        }

    }


    *out_lines = lines;
    *out_n = n;
    
    return ERR_OK;

}


static Err free_lines(app_context* app_context_param, char ***lines, size_t *n){

    if(!lines || !n) {

        APP_ERROR(app_context_param, "INVALID ARGUMENT");
        return ERR_INVALID_ARGUMENT;

    }

    for (size_t i = 0; i < *n; i++){

        if((*lines)[i])
            free((*lines)[i]);

    }

    free(*lines);
    *lines = NULL;
    *n = 0;

    return ERR_OK;

}


input_program* create_input_program(app_context* app_context_param, const char* input_file_path){

    if(!input_file_path){

        APP_ERROR(app_context_param, "INVALID ARGUMENT");
        return NULL;

    }

    FILE* fp_input_file = fopen(input_file_path, "r");

    if(!fp_input_file){

        APP_PERROR(app_context_param, "FILE CAN NOT BE OPENED.");
        return NULL;

    }

    input_program* input_prog = malloc(sizeof(input_program));

    if(!input_prog){

        fclose(fp_input_file);
        APP_PERROR(app_context_param, "MALLOC FAILED");
        return NULL;

    }

    input_prog->input = fp_input_file;
    input_prog->lines = NULL;
    input_prog->number_of_line = 0;

    return input_prog;
}

Err destroy_input_program(app_context* app_context_param, input_program* input_program_param){

    if(!input_program_param){

        APP_ERROR(app_context_param, "INVALID ARGUMENT");
        return ERR_INVALID_ARGUMENT;

    }

    fclose(input_program_param->input);

    Err e1;

    e1 = free_lines(app_context_param, &(input_program_param->lines), &(input_program_param->number_of_line));

    free(input_program_param);

    return e1;

}

Err print_lines(app_context* app_context_param, char** lines, size_t n){

    if(!lines || !(*lines)){

        APP_ERROR(app_context_param, "YOU CANNOT PRINT LINE(S): YOUR ARGUMENT INTO THIS FUNCTION IS INVALID.");
        return ERR_UB;

    }

    for(size_t i = 0; i < n; i++){

        printf("%s", lines[i]);

    }

    return ERR_OK;
}




