#include "regmap.h"
#include <string.h>


typedef struct{

    const char *name;
    int correspond_reg_num;

}reg_spec;


static const reg_spec reg_table[] = {

    {"$zero",  0},
    {"$at",    1},
    {"$v0",    2},
    {"$v1",    3},
    {"$a0",    4},
    {"$a1",    5},
    {"$a2",    6},
    {"$a3",    7},
    {"$t0",    8},
    {"$t1",    9},
   {"$t2",    10},
   {"$t3",    11},
   {"$t4",    12},
   {"$t5",    13},
   {"$t6",    14},
   {"$t7",    15},
   {"$s0",    16},
   {"$s1",    17},
   {"$s2",    18},
   {"$s3",    19},
   {"$s4",    20},
   {"$s5",    21},
   {"$s6",    22},
   {"$s7",    23},
   {"$t8",    24},
   {"$t9",    25},
   {"$k0",    26},
   {"$k1",    27},
   {"$gp",    28},
   {"$sp",    29},
   {"$fp",    30},
   {"$ra",    31}

};


int regmap_lookup(const char *s){

    if(!s) return -1;

    for(size_t i = 0; i < REG_FILE_SIZE; i++){

        if(strcmp(s, reg_table[i].name) == 0) return reg_table[i].correspond_reg_num;

    }

    return -1;   //invalid reg
}