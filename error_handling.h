#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H


typedef struct app_context_t app_context;

void app_error(app_context* app_context_param, const char* message, const char* file, int line, const char* func);

void app_perror(app_context* app_context_param, const char* message, const char* file, int line, const char* func);

#define APP_ERROR(app_context_param, message) app_error((app_context_param), (message), __FILE__, __LINE__, __func__);

#define APP_PERROR(app_context_param, message) app_perror((app_context_param), (message), __FILE__, __LINE__, __func__);

#endif