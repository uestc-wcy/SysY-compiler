#include "gen_arm2.h"
#include "ir_optimize.h"
#include "util.h"

extern std::vector<std::string>  global_decl_stmt;
extern std::list<IrFunction *> ir_functions;

std::map<std::string, GlobalSymbol> global_sym_table;

std::stringstream arm_head_info;
std::stringstream arm_decl_buffer;
std::stringstream arm_func_buffer;

std::string arm_code;

void gen_arm()
{
    gen_arm_head_info();
    gen_arm_global_var();
    for (IrFunction *function : ir_functions)
        gen_arm_func(function);
    arm_code = arm_head_info.str() + arm_func_buffer.str() + arm_decl_buffer.str();
}

void print_arm()
{
    std::cout << arm_code << "\n";
}

void gen_arm_head_info()
{

}

void gen_arm_global_var()
{
}

void gen_arm_func(IrFunction *function)
{
}