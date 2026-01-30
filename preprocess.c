#include "preprocess.h"
#include <string.h>
#include <ctype.h>

// preprocess part ***************************************

void strip_comment(char *s){

    char *p = strchr(s, '#');
    if(p) *p = '\0';

}

void trim_inplace(char *s){

    size_t i = 0;
    while(s[i] && isspace((unsigned char)s[i])) i++;

    if(i) memmove(s, s + i, strlen(s + i) + 1);

    size_t n = strlen(s);
    while(s[i] && isspace((unsigned char)s[n - 1])) s[--n] = '\0';


}

// ********************************************************