#ifndef GEN_ARM_H
#define GEN_ARM_H
using namespace std;

#include"ast.h"


void genline();
static void toarm(std::string str);
static void gen_globl_intdef(int mark, std::string str);
static void gen_constchar(std::string str);
static void gen_arrdef(std::string str);
static void gen_arrdef_non_init(std::string str);
static void gen_constarrdef(std::string str);
static void genlabel(std::string str);
static void gengoto(std::string str);
static void gen_if_stmt(std::string str);
static void genfunc(std::string str);
static void getreg(std::string str);
static bool is_in_reg_table(string str);
static void genexpress(std::string str);
static void gen_func_call(std::string str);
static void genreturn(std::string str);
static string mem_to_reg(int type, string var_name, int reg);
static int find_position(string var_name);

#endif//GEN_ARM_H