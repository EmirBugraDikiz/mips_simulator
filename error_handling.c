#include "error_handling.h"
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>


struct app_context_t{

    FILE* error_log;
    int last_error_num;
    char* error_log_path;

};

static FILE* error_sink(const app_context* app_context_param){                                                                  // this function decides where error logs will go.
    
    return (app_context_param)? app_context_param->error_log : stderr;

}

void app_error(app_context* app_context_param, const char* message, const char* file, int line, const char* func){              // use this function for logging errors that do not set errno.

    FILE* out = error_sink(app_context_param);
    fprintf(out, "[%s:%d:%s] %s\n", file, line, func, message);
    return;
}


void app_perror(app_context* app_context_param, const char* message, const char* file, int line, const char* func){             // use this function for logging erros that set errno.

    int e = errno;
    FILE* out = error_sink(app_context_param);
    fprintf(out, "[%s:%d:%s] %s: %s\n", file, line, func, message, strerror(e));
    if(app_context_param) app_context_param->last_error_num = e;

}

app_context* create_app_context(const char* error_log_path_param){                                                              // create app_context_t instance dynamically allocated.

    if(!error_log_path_param) {

        APP_ERROR(NULL, "invalid argument.");
        return NULL;
    }
    
    app_context* new_app_context = malloc(sizeof(app_context));

    if(!new_app_context) {

        APP_PERROR(NULL, "Allocation failed.");
        return NULL;
    }

    new_app_context->error_log_path = strdup(error_log_path_param);

    if(!new_app_context->error_log_path){

        APP_PERROR(NULL, "Allocation failed");
        free(new_app_context);
        return NULL;

    }

    new_app_context->error_log = fopen(error_log_path_param, "a");

    if(!new_app_context->error_log){
        APP_PERROR(NULL, "FILE cannot be opened.");
        free(new_app_context->error_log_path);
        free(new_app_context);
        return NULL;

    }


    new_app_context->last_error_num = 0;

    return new_app_context;
}




int destroy_app_context(app_context* app_context_param){                                                                        // deallocate app_context_t instance and close associated file pointer. 

    if(!app_context_param) return -1;    


    free(app_context_param->error_log_path);
    free(app_context_param);
    fclose(app_context_param->error_log);

    return 0;
}




