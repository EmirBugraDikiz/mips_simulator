#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H


typedef struct app_context_t app_context;   

typedef enum {

    ERR_OK = 0,             // Things work well.
    ERR_IO,                 // ERR_IO such as fopen() ...
    ERR_READ_ERROR,         // INDICATOR FOR READ ERROR. ferror flag setted. 
    ERR_EOF,                // END_OF_FILE REACHED BUT WE GOT SOME CHARACTER/S.
    ERR_NO_CHAR_INPUT,      // END_OF_FILE REACHED AND NO CHARACTER INPUT FOUND.
    ERR_OOM,                // OUT OF MEMORY such as malloc(), realloc(), calloc() fail.
    ERR_DEALLOC,            // DEALLOCATION FAIL.
    ERR_SYNTAX,             // 
    ERR_UNDEF_LABEL,        // 
    ERR_INVALID_ARGUMENT,   // INVALID FUNCTION ARGUMENTS. FOR EDGE CASE CONTROLS.
    ERR_UB                  // UNDEFINED BEHAVIOR.   
}Err;

void app_error(app_context* app_context_param, const char* message, const char* file, int line, const char* func);

void app_perror(app_context* app_context_param, const char* message, const char* file, int line, const char* func);

#define APP_ERROR(app_context_param, message) app_error((app_context_param), (message), __FILE__, __LINE__, __func__)

#define APP_PERROR(app_context_param, message) app_perror((app_context_param), (message), __FILE__, __LINE__, __func__)

app_context* create_app_context(const char* error_log_path_param);

Err destroy_app_context(app_context* app_context_param);

#endif