#ifndef GEN_ARM2_H
#define GEN_ARM2_H
#include<string>
#include "ir_optimize.h"

struct GlobalSymbol
{
    std::string name;
    std::string label;
    int offset; 
};

void gen_arm();
void print_arm();
void gen_arm_head_info();
void gen_arm_global_var();
void gen_arm_func(IrFunction *function);


#endif // GEN_ARM2_H