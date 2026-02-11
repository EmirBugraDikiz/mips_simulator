#include <stdio.h>
#include "core/ir.h"
#include "core/line.h"
#include "core/error_handling.h"
#include "asm/pass1.h"
#include "core/symtab.h"

const char *input_path = INPUT_PROG_PATH;
const char *log_path = ERROR_LOG_PATH;


int main(){

    Err e;
    app_context *ctx = create_app_context(log_path);
    if(!ctx) printf("app_context program cannot be created.\n");
    
    input_program *input_prog = create_input_program(ctx, input_path );
    
    e = read_all_lines(ctx, input_prog->input, &input_prog->lines, &input_prog->number_of_line);

    // print_lines(ctx, input_prog->lines, input_prog->number_of_line);

    const AsmConfig cfg = {0x00400000, 0x10010000};
    IR ir;
    Symtab symtab;
    AsmState final_state;
    
    e = assemble_pass1(ctx, &cfg, input_prog->lines, input_prog->number_of_line, &ir, &symtab, &final_state);

    if(e != ERR_OK){

        printf("pass1 failed\n");
        return 1;

    }

    ir_free(&ir, ctx);
    symtab_free(&symtab, ctx);



    e = destroy_input_program(ctx, input_prog);
    if(e != ERR_OK) printf("input_program");

    e = destroy_app_context(ctx);
    if(e != ERR_OK) printf("app_context program cannot be destroyed. There may be occure memory leak.\n");

    return 0;
    
}