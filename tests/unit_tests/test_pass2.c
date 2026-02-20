#include "asm/pass1.h"
#include "core/error_handling.h"
#include "core/symtab.h"
#include "test.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "core/ir.h"
#include "asm/pass2.h"


#define MAX_ASM_INPUT_LINES 25
#define MAX_LABELS_IN_ASM_INPUT 5
#define MAX_DATA_IN_ASM_INPUT   8

#define PACK_R(op, rs, rt, rd ,shamt, funct)\
  (  ((uint32_t)op << 26) | \
     ((uint32_t)rs << 21) | \
     ((uint32_t)rt << 16) | \
     ((uint32_t)rd << 11) | \
     ((uint32_t)shamt << 6) | \
     ((uint32_t)funct) )

    
#define PACK_I(op, rs, rt, imm)\
  (  ((uint32_t)op << 26) | \
     ((uint32_t)rs << 21) | \
     ((uint32_t)rt << 16) | \
     ((uint32_t)imm)  )


#define PACK_J(op, target26)\
  (  ((uint32_t)op << 26) | \
     (target26 & 0x03FFFFFFu) )

typedef struct{

    char *input_asm_lines[MAX_ASM_INPUT_LINES];
    uint32_t expected_text_image[MAX_ASM_INPUT_LINES];
    uint32_t expected_data_image[MAX_DATA_IN_ASM_INPUT];
    size_t expected_label_num;
    uint32_t expected_label_addresses[MAX_LABELS_IN_ASM_INPUT];
    Err expected_error_for_pass1;
    Err expected_error_for_pass2;
    
}pass2_case;

static pass2_case g_ok_cases[] = {

    {{".data",
                           "arr1: .word 10, 20, 30, 40, 50",
                           "arr2: .word -123, 125, 30",
                           "                         ",
                           ".text",
                           "main: add $s0, $s1, $s2",
                           "addu $t0, $s1, $t2",
                           "sub $sp, $s1, $s2",
                           "slt $s0, $t0, $t1",
                           "                 ",
                          "beq $s0, $s1, done",
                          "above: lw $s0, 4($s1)",
                          "             ",
                          "and $a1, $a2, $v1",
                          "done: j above",
                          "srl $s0, $k1, 23",
                          "ori $t0, $a1, 1000",
                          "slti $s0, $fp, -245",
                          "",
                          "",
                          "",
                          "",
                          "",
                          "",
                          ""},
  {PACK_R(0, 17, 18, 16, 0, 0X20), PACK_R(0, 17, 10, 8, 0, 0x21), PACK_R(0, 17, 18, 29, 0, 0x22),
                           PACK_R(0, 8, 9, 16, 0, 0x2A), PACK_I(0x04, 16, 17, 2), PACK_I(0x23, 17, 16, 4),
                           PACK_R(0, 6, 3, 5, 0, 0x24), PACK_J(0x02, 0x00100005), PACK_R(0, 0, 27, 16, 23, 0x02),
                           PACK_I(0x0D, 5, 8, 1000), PACK_I(0x0A, 30, 16, 0xff0b /* this 0xff0b means -245 in signed representation*/)},
  {10,20,30,40,50,-123,125,30},
           0,
{0x10010000, 0x10010014,0x00400000, 0x00400014, 0x0040001C}, ERR_OK, ERR_OK
        }


};


uint16_t a = 0xff0b;
int16_t b = 0xff0b;

size_t count_instr_num(IR *ir){

    size_t instr_count = 0;

    for(size_t i = 0; i < ir->n; i++){

        if(ir->v[i].kind == ST_INSTR || ir->v[i].kind == ST_LABEL_PLUS_INSTR) instr_count++;

    }

    return instr_count;

}


static void run_pass2_case(app_context *app_context_param , pass2_case *test_case){

    AsmConfig cfg = {0x00400000, 0X10010000};
    AsmState final_state;
    
    IR ir;
    Symtab symtab;

    Err e;

    e = assemble_pass1(app_context_param, &cfg, test_case->input_asm_lines , MAX_ASM_INPUT_LINES, &ir, &symtab, &final_state);

    ASSERT_EQ_INT(e, test_case->expected_error_for_pass1);

    for(size_t i = 0; i < MAX_LABELS_IN_ASM_INPUT; i++){

        ASSERT_EQ_INT(test_case->expected_label_addresses[i], symtab.v[i].addr);

    }

    fprintf(stderr, "DEBUG: label and instruction number before assembler pass2: %lu  %lu\n", symtab.n, count_instr_num(&ir));

    AssemblerOutput out_assembler;

    e = assemble_pass2(app_context_param, &cfg, &final_state, &ir, &symtab, &out_assembler);

    if(e != ERR_OK){

        ir_free(&ir, app_context_param);
        symtab_free(&symtab, app_context_param);
        
    }

    ASSERT_EQ_INT(e, test_case->expected_error_for_pass2);

    for(size_t i = 0; i < out_assembler.text_words; i++){

        fprintf(stderr, "text image comparison loop\n");
        ASSERT_EQ_UINT32_T(test_case->expected_text_image[i], out_assembler.text_image[i]);

    }


    for(size_t i = 0; i < MAX_DATA_IN_ASM_INPUT; i++){

        ASSERT_EQ_UINT32_T(test_case->expected_data_image[i], out_assembler.data_image[i]);

    }
    

    assembler_output_free(&out_assembler, app_context_param);


}

static void run_pass2_cases( pass2_case *cases, size_t n, app_context *app_context_param){

    for(size_t i = 0; i < n; i++){

        run_pass2_case(app_context_param, &cases[i]);

    }

}


void test_all_pass2_cases(app_context *app_context_param){

    run_pass2_cases(g_ok_cases, ARR_LEN(g_ok_cases), app_context_param);

}



/*static encoder_case ok_unit_cases[] = {

    {"add $t0, $t1, $t2", 0, NULL, 0, SEC_NONE, ERR_OK, PACK_R(0x00, 0x09, 0x0A, 0x08, 0x00, 0x20)},
    {"addu $s0, $s1, $s2", 0, NULL, 0, SEC_NONE, ERR_OK, PACK_R(0x00, 0x11, 0x12, 0x10, 0x00, 0x21)},
    {"sub $t0, $gp, $t2", 0, NULL, 0, SEC_NONE, ERR_OK, PACK_R(0x00, 0x1C, 0x0A, 0x08, 0x00, 0x22)},
    {"subu $s3, $sp, $gp", 0, NULL, 0, SEC_NONE, ERR_OK, PACK_R(0X00, 0X1D, 0X1C, 0X13, 0x00, 0x23)},
    {"and $s3, $sp, $gp", 0, NULL, 0, SEC_NONE, ERR_OK, PACK_R(0x00, 0x1D, 0x1C, 0x13, 0x00, 0x24)},
    {"or $s0, $s1, $s2", 0, NULL, 0, SEC_NONE, ERR_OK, PACK_R(0x00, 0x11, 0x12, 0x10, 0x00, 0x25)},
    {"xor $s0, $s1, $s2", 0, NULL, 0, SEC_NONE, ERR_OK, PACK_R(0x00, 0x11, 0x12, 0x10, 0x00, 0x26)},
    {"nor $s0, $s1, $s2", 0, NULL, 0, SEC_NONE, ERR_OK, PACK_R(0x00, 0x11, 0x12, 0x10, 0x00, 0x27)},
    {"slt $s0, $s1, $s2", 0, NULL, 0, SEC_NONE, ERR_OK, PACK_R(0x00, 0x11, 0x12, 0x10, 0x00, 0x2A)},
    {"sltu $s0, $s1, $s2", 0, NULL, 0, SEC_NONE, ERR_OK, PACK_R(0x00, 0x11, 0x12, 0x10, 0x00, 0x2B)},
    {"sllv $s0, $s1, $s2", 0, NULL, 0, SEC_NONE, ERR_OK, PACK_R(0x00, 0x11, 0x12, 0x10, 0x00, 0x04)},
    {"srlv $s0, $s1, $s2", 0, NULL, 0, SEC_NONE, ERR_OK, PACK_R(0x00, 0x11, 0x12, 0x10, 0x00, 0x06)},
    {"srav $s0, $s1, $s2", 0, NULL, 0, SEC_NONE, ERR_OK, PACK_R(0x00, 0x11, 0x12, 0x10, 0x00, 0x07)},

    {"sll $s0, $s1, 31", 0, NULL, 0, SEC_NONE, ERR_OK, PACK_R(0x00, 0x00, 0x11, 0x10, 0x001F, 0x00)},
    {"srl $s0, $s1, 14", 0, NULL, 0, SEC_NONE, ERR_OK, PACK_R(0x00, 0x00, 0x11, 0x10, 0x000E, 0x02)},
    {"sra $s0, $s1, 14", 0, NULL, 0, SEC_NONE, ERR_OK, PACK_R(0x00, 0x00, 0x11, 0x10, 0x000E, 0x03)},

    {"addi $s0, $s1, 32760", 0, NULL, 0, SEC_NONE, ERR_OK, PACK_I(0x08, 0x11, 0x10, 0x7FF8)},
    {"addiu $s0, $s1, -32700", 0, NULL, 0, SEC_NONE, ERR_OK, PACK_I(0x09, 0x11, 0x10, 0x8044)},
    {"andi $s0, $s1, 250", 0, NULL, 0, SEC_NONE, ERR_OK, PACK_I(0x0C, 0x11, 0x10, 0x00FA)},
    {"ori $s0, $s1, 125", 0, NULL, 0, SEC_NONE, ERR_OK,PACK_I(0x0D, 0x11, 0x10, 0x007D)},
    {"xori $s0, $s1, 2012", 0, NULL, 0, SEC_NONE, ERR_OK,  PACK_I(0x0E, 0x11, 0x10, 0x07DC)},
    {"slti $s0, $s1, -4231", 0, NULL, 0, SEC_NONE, ERR_OK, PACK_I(0x0A, 0x11, 0x10, 0xEF79)},
    {"sltiu $s0, $s1, -1235", 0, NULL, 0, SEC_NONE, ERR_OK, PACK_I(0x0B, 0x11, 0x10, 0xFB2D)},

    {"lw $t2, 32($0)", 0, NULL, 0, SEC_NONE, ERR_OK, PACK_I(0x23, 0x00, 0x0A, 0x0020)},
    {"sw $s1, 4($t1)", 0, NULL, 0, SEC_NONE, ERR_OK, PACK_I(0x2B, 0x09, 0x11, 0x0004)},

    {"beq $s0, $s1, branch_label", 0X00400000, "branch_label", 0x00400008, SEC_TEXT, ERR_OK, PACK_I(0X04, 0x11, 0x10, 0X0001)},

    {"j another_label", 0x00400000, "another_label", 0x00400020, SEC_TEXT, ERR_OK, PACK_J(0x02, 0x00100008)},
    {"jal another_label", 0x00400000, "another_label", 0x00400020, SEC_TEXT, ERR_OK, PACK_J(0X03, 0x00100008)}

};
*/