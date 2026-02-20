#include "test.h"
#include <string.h>




int main(){

    
    test_preprocess_all_tables();
    test_lex_all_tables(NULL);
    test_parser_tables(NULL);
    test_pass1_tables(NULL);
    test_all_pass2_cases(NULL);
    
    return 0;
}