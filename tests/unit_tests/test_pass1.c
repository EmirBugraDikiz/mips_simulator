#include "test.h"
#include "asm/pass1.h"
#include "core/ir.h"
#include "core/error_handling.h"
#include <assert.h>
#include <stdint.h>


#define INPUT_PROGRAM_SIZE 10


typedef struct{


    const char *name;
    char *lines[INPUT_PROGRAM_SIZE];    // this is the program that will be tested and it contains INPUT_PROGRAM_SIZE line
    uint32_t label_addresses[3];              // the input program that will be used in this test can contain maximum 3 label for test simplicity. 
    AsmState expected_final_state;


}pass1_case;


static void run_pass1_case(app_context *app_context_param, pass1_case* test_case){

    const AsmConfig cfg = {0x00400000, 0x10010000};
    AsmState state;
    IR ir;
    Symtab symtab;
    Err e;

    e = assemble_pass1(app_context_param, &cfg, test_case->lines, INPUT_PROGRAM_SIZE, &ir, &symtab, &state);

    if(e != ERR_OK){

        fprintf(stderr, "\n[CASE]  name = %s\n", test_case->name);

    }

    ASSERT_EQ_INT(e, ERR_OK);
    ASSERT_EQ_INT(state.data_pc, test_case->expected_final_state.data_pc);
    ASSERT_EQ_INT(state.text_pc, test_case->expected_final_state.text_pc);
    

    for(size_t i = 0; i < sizeof(test_case->label_addresses) / sizeof(test_case->label_addresses[0]); i++){

        ASSERT_EQ_INT(symtab.v[i].addr, test_case->label_addresses[i]);

    }

    ir_free(&ir, app_context_param);
    symtab_free(&symtab, app_context_param);


}


static pass1_case pass1_table[] = {

    {"test_input_program1",
        {".data",
                "a: .word 10, 30, 0x10, -135",
                ".text",
                "main: add $t0, $t1, $t2",
                "j instruction_label",
                "instruction_label:",
                "addi $t1, $t2, 0xFF",
                "",
                "",
                ""},
                {0x10010000, 0x00400000, 0x00400008},
                {SEC_TEXT, 12, 16}},
    
    {"test_input_program2",
        {".text",
                "main: lw $t0, 8($sp)",
                "beq $s0, $t1, branch",
                "branch: addi $t2, $t3, 0xFF",
                ".data",
                "array1: .word 25, -32, 0x00FFA00B",
                "",
                "",
                "",
                ""},
                {0x00400000, 0x00400008, 0x10010000},
                {SEC_DATA, 12, 12}}
            
};


void test_pass1_tables(app_context *app_context_param){

    for(size_t i = 0; i < ARR_LEN(pass1_table); i++){

        run_pass1_case(app_context_param, &pass1_table[i]);

    }

}