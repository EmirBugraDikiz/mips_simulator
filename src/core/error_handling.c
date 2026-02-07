#include "core/error_handling.h"
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


void app_perror(app_context* app_context_param, const char* message, const char* file, int line, const char* func){             // use this function for logging errors that set errno.

    int e = errno;
    FILE* out = error_sink(app_context_param);
    fprintf(out, "[%s:%d:%s] %s: %s\n", file, line, func, message, strerror(e));
    if(app_context_param) app_context_param->last_error_num = e;

}

app_context* create_app_context(const char* error_log_path_param){                                                              // create app_context_t instance dynamically allocated.

    if(!error_log_path_param) {

        APP_ERROR(NULL, "INVALID ARGUMENT");
        return NULL;
    }
    
    app_context* new_app_context = malloc(sizeof(app_context));

    if(!new_app_context) {

        APP_PERROR(NULL, "APP CONTEXT STRUCT: ALLOCATION FAILED");
        return NULL;
    }

    new_app_context->error_log_path = strdup(error_log_path_param);

    if(!new_app_context->error_log_path){

        APP_ERROR(NULL, "APP CONTEXT STRUCT MEMBER NOT ALLOCATED: STRDUP FAILED.");
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




Err destroy_app_context(app_context* app_context_param){                                                                        // deallocate app_context_t instance and close associated file pointer. 

    if(!app_context_param) {

        APP_ERROR(app_context_param, "invalid argument.");
        return ERR_INVALID_ARGUMENT;
    }    

    
    fclose(app_context_param->error_log);
    free(app_context_param->error_log_path);
    free(app_context_param);
    

    return ERR_OK;
}




