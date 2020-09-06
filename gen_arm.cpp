#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <sstream>
#include "gen_arm.h"

using namespace std;

static std::string ir_code;
extern std::string optimized_ir_code;

//用于对中间代码进行匹配的字符串
static std::string label = "label";
static std::string go_to = "goto";
static std::string func = "function";
static std::string intdef = "var_int_init";
static std::string constintdef = "const_int_init";
static std::string int_non_init_def = "int_non_init";
static std::string return_stmt = "return";
static std::string constarrdef = "const_array_init";
static std::string if_stmt = "if";
static std::string arrdef = "var_array_init";
static std::string arrdef_non_init = "array_non_init";
static std::string constchar = "ascii";
static std::string call_func = "call";

//汇编程序的各个组成部分
static string code_head; //汇编程序头部
static string value_def; //全局变量的声明
static string array_def; //数组的声明定义，包括局部数组和全局数组
static string arm_code;  //汇编程序代码

static int ik = 0; //用于区分一些临时标号的名称

static string getreg_arm_code; //给变量分配内存的arm代码
static string sub_sp;          //每个函数需要的堆栈空间 在翻译return时可计算得出 然后输出在函数标签后
static string func_def_arm_code;
static int sp = 4; //用于记录变量分配堆栈的情况

static bool is_in_main = false;
static bool is_div = false; //判断程序中是否有除法

struct Var_Stack_Table
{
    string var_name; //变量名
    int stack_pos;   //给变量分配的内存空间的位置
};

static std::vector<Var_Stack_Table *> reg_table; //记录变量名和对应在内存的位置

//一行一行地读取中间代码
void genline()
{
    ir_code = optimized_ir_code;
    stringstream head_code;
    head_code << "\t.arch armv7-a"
              << endl;
    head_code << "\t.eabi_attribute 28, 1\n\t.eabi_attribute 20, 1\n\t.eabi_attribute 21, 1\n\t.eabi_attribute 23, 3\n\t.eabi_attribute 24, 1\n\t.eabi_attribute 25, 1\n\t.eabi_attribute 26, 2\n\t.eabi_attribute 30, 6\n\t.eabi_attribute 34, 1\n\t.eabi_attribute 18, 4\n";
    head_code << "\t.data\n";

    code_head = head_code.str();

    int size = ir_code.length();
    int head = 0;
    int index = 0;
    while (head != size)
    {
        if (head == 0)
        {
            index = ir_code.find("\n");
        }
        else
        {
            index = ir_code.find("\n", head);
        }
        string str = ir_code.substr(head, index - head);
        toarm(str);
        index++;
        head = index;
    }

    cout << code_head;
    cout << value_def;
    cout << array_def;
    cout << "\t.text\n";
    if (is_div == true)
        cout << "\t.globl __aeabi_idiv"
             << "\n"
             << "\t.globl __aeabi_idivmod"
             << "\n";

    cout << arm_code << "\n";
}

//逐行对中间代码进行翻译
void toarm(std::string str)
{

    int index = str.find(" ");
    string s = str.substr(0, index);
    if (label.compare(s) == 0)
    {
        genlabel(str);
    }
    else if (func.compare(s) == 0)
    {
        genfunc(str);
    }
    else if (intdef.compare(s) == 0)
    {
        gen_globl_intdef(1, str);
    }
    else if (constintdef.compare(s) == 0)
    {
        gen_globl_intdef(2, str);
    }
    else if (int_non_init_def.compare(s) == 0)
    {
        gen_globl_intdef(3, str);
    }
    else if (str.substr(0, 7) == "@.ascii")
    {
        //gen_constchar(str);
    }
    else if (s[0] == '%' || s[0] == '@')
    {
        getreg(str);
    }
    else if (return_stmt.compare(s) == 0)
    {
        genreturn(str);
    }
    else if (str.find(if_stmt) != str.npos)
    {
        gen_if_stmt(str);
    }
    else if (arrdef.compare(s) == 0)
    {
        gen_arrdef(str);
    }
    else if (arrdef_non_init.compare(s) == 0)
    {
        gen_arrdef_non_init(str);
    }
    else if (constarrdef.compare(s) == 0)
    {
        gen_constarrdef(str);
    }
    else if (go_to.compare(s) == 0)
    {
        gengoto(str);
    }
    else if (call_func.compare(s) == 0)
    {
        gen_func_call(str);
    }
}

//用于全局已初始化或为初始化变量的代码生成
void gen_globl_intdef(int mark, std::string str) //mark用于标记是已初始化还是未初始化 1,2是已初始化，3是未初始化
{
    stringstream valuedef;
    int index = str.find(" ");
    int head = index;
    int tail = str.find("=");
    string s = str.substr(head, tail - head);

    valuedef << "\t.globl " + s.substr(2) << "\n";
    valuedef << s.substr(2) << ":\n\t"
             << ".word";

    if (mark == 3)
    {
        valuedef << "\t0\n\n";
    }
    else
    {
        string value = str.substr(tail + 1);
        valuedef << "\t" << value << "\n";
    }

    value_def = value_def + valuedef.str();
}

void gen_constchar(std::string str)
{
    stringstream valuedef;
    int index = str.find("\"");
    string s = str.substr(index + 1, str.size() - index - 2);
    index = str.find(" ") - 1;
    string s1 = str.substr(index, 1);
    valuedef << ".LSPC" + s1 + ":" << endl;
    valuedef << "\t.ascii    " + s << endl;

    value_def = value_def + valuedef.str();
}

void gen_arrdef(std::string str)
{
    stringstream arraydef;
    int index = str.find(" ");
    int head = index;
    int tail = str.find("=");
    string s = str.substr(head + 2, tail - head - 3);
    arraydef << "\t.globl " + s << endl;
    arraydef << "\t.type    " + s + ", %object" << endl;

    arraydef << s + ":" << endl;
    int sta = str.find("{");
    ;
    int end = str.find("}");
    ;
    int lastcomma = str.find_last_of(",");
    string s1 = str.substr(sta + 1, end - sta - 1);
    s1 = s1 + ",";
    //cout <<s1<< endl;
    int size = s1.length() - 1;
    sta = 0;
    end = s1.find(",");

    std::string s2 = s1.substr(sta, end - sta);

    while (end != size)
    {
        std::string s2 = s1.substr(sta, end - sta);

        if (s2.find(".") == s2.npos)
        {
            arraydef << "\t.word  " + s2 << endl;
        }
        else
        {
            sta = s2.find(".");
            std::string s3 = s2.substr(sta + 1, end);
            int intStr = atoi(s3.c_str());

            if (intStr > 0)
                arraydef << "\t.space  " + to_string(intStr) << endl;
        }
        sta = end + 1;
        end = s1.find(",", sta);
    }
    std::string s4 = s1.substr(sta, end - sta);
    if (s4.find(".") == s4.npos)
    {
        arraydef << "\t.word  " + s4 << endl;
    }
    else
    {
        sta = s4.find(".");
        std::string s5 = s4.substr(sta + 1, end);
        int intm = atoi(s5.c_str());

        if (intm > 0)
            arraydef << "\t.space  " + to_string(intm) << endl;
    }

    array_def = array_def + arraydef.str();
}

void gen_arrdef_non_init(std::string str)
{
    stringstream arraydef;
    int index = str.find(" ");
    int head = index;
    index = str.find("=");
    string s = str.substr(head + 2, index - head - 3);
    arraydef << "\t.globl " + s << endl;
    arraydef << "\t.type    " + s + ", %object" << endl;

    arraydef << s + ":" << endl;
    int sta = str.find("[");
    int end = str.find("]");

    string s1 = str.substr(sta + 1, end - sta - 1);
    int intStr = atoi(s1.c_str());

    if (intStr > 0)
        arraydef << "\t.space  " << intStr * 4 << endl;

    array_def = array_def + arraydef.str();
}

void gen_constarrdef(std::string str)
{
    stringstream arraydef;
    int index = str.find(" ");
    int head = index;
    int tail = str.find("=");
    string s = str.substr(head + 2, tail - head - 3);
    arraydef << "\t.globl " + s << endl;
    arraydef << "\t.type    " + s + ", %object" << endl;

    arraydef << s + ":" << endl;
    int sta = str.find("{");
    int end = str.find("}");
    int lastcomma = str.find_last_of(",");
    string s1 = str.substr(sta + 1, end - sta - 1);
    s1 = s1 + ",";
    //cout <<s1<< endl;
    int size = s1.length() - 1;
    sta = 0;
    end = s1.find(",");

    std::string s2 = s1.substr(sta, end - sta);

    while (end != size)
    {
        std::string s2 = s1.substr(sta, end - sta);

        if (s2.find(".") == s2.npos)
        {
            arraydef << "\t.word  " + s2 << endl;
        }
        else
        {
            sta = s2.find(".");
            std::string s3 = s2.substr(sta + 1, end);
            int intStr = atoi(s3.c_str());
            if (intStr > 0)
                arraydef << "\t.space  " + to_string(intStr) << endl;
        }
        sta = end + 1;
        end = s1.find(",", sta);
    }
    std::string s4 = s1.substr(sta, end - sta);
    if (s4.find(".") == s4.npos)
    {
        arraydef << "\t.word  " + s4 << endl;
    }
    else
    {
        sta = s4.find(".");
        std::string s5 = s4.substr(sta + 1, end);
        int intm = atoi(s5.c_str());
        if (intm > 0)
            arraydef << "\t.space  " + to_string(intm) << endl;
    }

    array_def = array_def + arraydef.str();
}

void genlabel(std::string str)
{
    int index = str.find(" ");
    int head = index + 1;
    index = str.find(":");
    string s = str.substr(head, index);
    stringstream code;
    code << s << ":\n";
    getreg_arm_code = getreg_arm_code + code.str();
}
void gengoto(std::string str)
{
    int index = str.find(" ");
    int head = index + 1;
    string s = str.substr(head);
    stringstream code;
    code << "\tb     " << s << "\n";
    getreg_arm_code = getreg_arm_code + code.str();
}

void gen_if_stmt(std::string str)
{
    string left_var;
    string right_var;

    int index, tail;
    index = str.find(" ");
    tail = str.find(" ", index + 1);
    left_var = str.substr(index + 1, tail - index - 1);
    index = str.find(" ", tail + 1);
    int index2 = str.find("goto");
    tail = index2;
    right_var = str.substr(index + 1, tail - index - 2);

    stringstream code;

    code << mem_to_reg(0, left_var, 3);
    code << mem_to_reg(0, right_var, 4);

    index = str.find("goto") + 5;
    string sys = str.substr(index);

    if (str.find(">") != str.npos && str.find(">=") == str.npos)
    {
        code << "\tcmp r3,r4" << endl;
        code << "\tbgt " + sys << endl;
    }
    else if (str.find("<") != str.npos && str.find("<=") == str.npos)
    {
        code << "\tcmp r3,r4" << endl;
        code << "\tblt " + sys << endl;
    }
    else if (str.find("!=") != str.npos)
    {
        code << "\tcmp r3,r4" << endl;
        code << "\tbne " + sys << endl;
    }
    else if (str.find("<=") != str.npos)
    {
        code << "\tcmp r3,r4" << endl;
        code << "\tble " + sys << endl;
    }
    else if (str.find(">=") != str.npos)
    {
        code << "\tcmp r3,r4" << endl;
        code << "\tbge " + sys << endl;
    }
    else if (str.find("==") != str.npos)
    {
        code << "\tcmp r3,r4" << endl;
        code << "\tbeq " + sys << endl;
    }

    getreg_arm_code = getreg_arm_code + code.str();
}

void genfunc(std::string str)
{
    stringstream def_code;
    int index = str.find(" ");
    int head = index;
    int tail = str.find("(");
    string s = str.substr(head + 1, tail - head - 1);
    if (s.compare("main") == 0) //不是main函数的话，不需要加全局声明
    {
        def_code << "\t.globl " + s << endl;
        is_in_main = true;
    }

    def_code << "\t.syntax unified" << endl;
    def_code << "\t.type    " + s + ", %function" << endl;
    def_code << s + ":" << endl;

    //生成入栈语句，调整sp与r7的值
    std::stringstream push_code;
    def_code << "\tpush {r7, lr}\n";
    func_def_arm_code = def_code.str();

    //生成传参语句(参数也作为变量在堆栈中分配空间)
    string para = str.substr(tail + 1, str.size() - tail - 3) + ",";
    string var_name;
    head = 0;
    int i = 0;
    while (head != para.length())
    {
        if (head == 0)
        {
            index = para.find(",");
        }
        else
        {
            index = para.find(",", head);
        }
        var_name = para.substr(head, index - head);
        if (var_name == "")
            break;
        if (var_name.find('[') != var_name.npos)
        {
            int tem = var_name.find('[');
            var_name = var_name.substr(1, tem - 1);
        }
        index++;
        head = index;
        //判断是否超过四个参数
        if (i > 3)
        {
            push_code << "\tldr r3, [r4, #-" << (i - 3) * 4 << "]\n";
            push_code << "\tstr r3, [r7, #" << sp << "]\n";
        }
        else
        {
            push_code << "\tstr r" << i << ", [r7, #" << sp << "]\n";
        }
        Var_Stack_Table *var = new Var_Stack_Table();
        var->stack_pos = sp;
        var->var_name = var_name;
        reg_table.push_back(var);
        i++;
        sp += 4;
    }
    getreg_arm_code = getreg_arm_code + push_code.str();
}

//处理等式，为等式左边变量分配内存并处理等式右边的表达式
void getreg(std::string str)
{
    int index, head, tail;

    //先判断是否是为局部数组变量分配空间
    if (str.find("alloca") != str.npos && str.find("alloca(") == str.npos)
    {
        head = str.find('#');
        string size = str.substr(head);

        index = str.find("%");
        head = index;
        tail = str.find(" ");
        string var_name = str.substr(head + 1, tail - head - 1);

        //在堆栈上给数组变量分配一个空间，用于存储数组的地址
        Var_Stack_Table *var = new Var_Stack_Table();

        stringstream code;
        code << mem_to_reg(0, size, 0);
        code << "\tadd  r0, r0, #8\n";
        code << "\tsub  sp, sp, r0\n";
        code << "\tadd  r3, sp, #4\n";

        var->stack_pos = sp;
        var->var_name = var_name;
        reg_table.push_back(var);
        sp += 4;

        code << "\tstr  r3, [r7, #" << find_position(var_name) << "]\n";
        getreg_arm_code = getreg_arm_code + code.str();
        return;
    }
    else if (str.find("alloca(") != str.npos)
    {
        index = str.find("%");
        head = index;
        tail = str.find(" ");
        string var_name = str.substr(head, tail - head);

        stringstream code;

        Var_Stack_Table *var = new Var_Stack_Table();
        var->stack_pos = sp;
        sp += 4;
        var->var_name = var_name;
        reg_table.push_back(var);

        code << "\tmov  r0, #0\n";
        code << "\tstr  r0, [r7, #" << find_position(var_name) << "]\n";
        getreg_arm_code = getreg_arm_code + code.str();
        return;
    }

    //判断是否是数组参数传递
    if (str.find("&") != str.npos)
    {
        //判断取的数组地址是不是全局地址
        if (str.find("@") != str.npos)
        {
            index = str.find("%");
            head = index;
            tail = str.find(" ");
            string var_name = str.substr(head, tail - head);
            index = str.find("@");
            string array_name = str.substr(index + 1);
            stringstream code;

            Var_Stack_Table *var = new Var_Stack_Table();
            var->stack_pos = sp;
            sp += 4;
            var->var_name = var_name;
            reg_table.push_back(var);

            code << "\tldr r3, =" << array_name << "\n";
            code << "\tb .LIC" << ik
                 << "\n";
            code << "\t.ltorg"
                 << "\n";
            code << ".LIC" << ik << ":"
                 << "\n";
            ik++;
            code << "\tstr r3, [r7, #" << find_position(var_name) << "]\n";
            getreg_arm_code = getreg_arm_code + code.str();
            return;
        }
        else
        {
            index = str.find("%");
            head = index;
            tail = str.find(" ");
            string var_name = str.substr(head, tail - head);
            index = str.find("%", head + 1);
            string array_name = str.substr(index + 1);
            stringstream code;

            Var_Stack_Table *var = new Var_Stack_Table();
            var->stack_pos = sp;
            sp += 4;
            var->var_name = var_name;
            reg_table.push_back(var);

            code << "\tldr r3, [r7, #" << find_position(array_name) << "]\n";
            code << "\tstr r3, [r7, #" << find_position(var_name) << "]\n";
            getreg_arm_code = getreg_arm_code + code.str();
            return;
        }
    }

    string var_name;
    if (str[0] == '%')
    {
        index = str.find("%");
        head = index;
        tail = str.find(" ");
        var_name = str.substr(head, tail - head);
        //判断等式左边是否是数组的一项，是的话不用分配内存
        if (var_name.find('[') == var_name.npos && !is_in_reg_table(var_name))
        {
            Var_Stack_Table *var = new Var_Stack_Table();
            var->stack_pos = sp;
            sp += 4;
            var->var_name = var_name;
            reg_table.push_back(var);
        }
    }
    else if (str[0] == '@')
    { //全局变量就先不放入内存分配表  直接判断等号右边的结果
        index = str.find("@");
        head = index;
        tail = str.find(" ");
        var_name = str.substr(head, tail - head);
    }

    //判断如果是表达式或赋值语句的话要生成相应码?
    head = str.find("=");
    head = str.find(" ", head);
    tail = str.find("\n");
    string right_value = str.substr(head + 1, tail - head);
    genexpress(str);
}

//判断变量是否在内存分配表上
bool is_in_reg_table(string str)
{
    for (int i = 0; i < reg_table.size(); i++)
    {
        if (str == reg_table[i]->var_name)
            return true;
    }
    return false;
}

//用于处理表达式的代码生成
void genexpress(std::string str)
{
    string left_var;
    string right_expr;
    string right_var1, right_var2; //右边表达式一定是二元式或一元式或函数调�?
    string Operator;
    std::stringstream assign_code;
    int position;

    int index, head;
    index = str.find(" ");
    left_var = str.substr(0, index);
    head = str.find("=");
    index = str.find("\n");
    right_expr = str.substr(head + 2, index - head - 1);

    index = right_expr.find(" ");
    if (index == right_expr.npos)
    { //说明是一元表达式
        right_var1 = right_expr.substr(0);
        assign_code << mem_to_reg(0, right_var1, 3);
    }
    else if (right_expr.substr(0, index) == "call")
    { //说明是函数调用
        gen_func_call(right_expr);
    }
    else
    { //说明是二元表达式
        right_var1 = right_expr.substr(0, index);
        if (right_var1 == "-")
        { //说明是表达式 - %x或 -@x
            index = right_expr.find(" ");
            right_var2 = right_expr.substr(index + 1);

            assign_code << mem_to_reg(0, right_var2, 4);

            assign_code << "\tmov r5, #0\n";
            assign_code << "\tsub r3, r5, r4"
                        << "\n";
        }
        else
        {
            Operator = right_expr.substr(index + 1, 1);
            index = right_expr.find(" ", index + 1);
            right_var2 = right_expr.substr(index + 1);

            if (right_var1 == "alloca")
            {
                return;
            }

            assign_code << mem_to_reg(0, right_var1, 3);
            assign_code << mem_to_reg(0, right_var2, 4);

            //判断操作符的类型
            if (Operator == "+")
            {
                assign_code << "\tadd r3, r3, r4\n";
            }
            else if (Operator == "-")
            {
                assign_code << "\tsub r3, r3, r4\n";
            }
            else if (Operator == "*")
            {
                assign_code << "\tmul r3, r3, r4\n";
            }
            else if (Operator == "m")
            {
                is_div = true;
                assign_code << "\tmov r0, r3\n"
                            << "\tmov r1, r4\n"
                            << "\tbl __aeabi_idivmod(PLT)\n"
                            << "\tmov r3, r1\n";
            }
            else if (Operator == "/")
            {
                is_div = true;
                assign_code << "\tmov r0, r3\n"
                            << "\tmov r1, r4\n"
                            << "\tbl __aeabi_idiv(PLT)\n"
                            << "\tmov r3, r0\n";
            }
        }
    }

    assign_code << mem_to_reg(1, left_var, 3);

    getreg_arm_code = getreg_arm_code + assign_code.str();
}

//用于处理函数调用
void gen_func_call(std::string str)
{
    bool more_than_four_param = false;
    stringstream param_code;
    string func_name;
    string func_param;
    int index, head, tail;
    index = str.find(" ");
    head = index;
    tail = str.find("(");
    func_name = str.substr(head, tail - head);
    func_param = str.substr(tail + 1, str.size() - tail - 2) + ",";

    // 将参数依次放入寄存器
    string var_name;
    head = 0;
    int i = 0;
    int postision;
    while (head != func_param.length())
    {
        if (head == 0)
        {
            index = func_param.find(",");
        }
        else
        {
            index = func_param.find(",", head);
        }
        var_name = func_param.substr(head, index - head);
        if (var_name[0] == ' ')
            var_name = var_name.substr(1);
        index++;
        head = index;
        //判断是否超过四个参数
        if (i > 3)
        {
            more_than_four_param = true;

            param_code<<mem_to_reg(0,var_name,4);
            param_code << "\tstr r4, [sp, #-" << (i - 3) * 4 << "]\n";
        }
        else
        { 
            param_code << mem_to_reg(0, var_name, i); 
        }
        i++;
    }
    if (more_than_four_param)
    {
        param_code << "\tmov r4, sp"
                   << "\n";
    }

    if (func_name == " putf")
        func_name = "printf";

    if (func_name == " stoptime")
        func_name = "_sysy_stoptime";

    if (func_name == " starttime")
        func_name = "_sysy_starttime";

    if (more_than_four_param)
    {
        param_code << "\tsub sp,sp,#" << (i - 2) * 4 << "\n";
        param_code << "\tbl   " << func_name << "(PLT)\n";
        param_code << "\tmov r3, r0\n";
        param_code << "\tadd sp,sp,#" << (i - 2) * 4 << "\n";
    }
    else
    {
        param_code << "\tbl   " << func_name << "(PLT)\n";
        param_code << "\tmov r3, r0\n";
    }

    getreg_arm_code = getreg_arm_code + param_code.str();
}

//return时打印所有缓冲区中的语句 并且清空缓冲区和内存分配�?
void genreturn(std::string str)
{
    //首先计算一个函数所需要的堆栈空间
    std::stringstream change_sp;
    change_sp << "\tsub    sp, sp, #" << sp + 8
              << "\n"
              << "\tadd    r7, sp, #0\n";
    sub_sp = change_sp.str();

    arm_code = arm_code + func_def_arm_code + sub_sp + getreg_arm_code;
    //生成传递返回值的语句
    int index, head;
    head = str.find(" ");
    index = str.find("\n");
    string return_var = str.substr(head + 1);
    if (return_var != "")
    {
        stringstream return_code;
        return_code <<mem_to_reg(0,return_var,3);
        return_code << "\tmov r0, r3\n"
                    << "\tmov sp, r7\n"
                    << "\tadd  sp, sp, #" << sp + 8
                    << "\n"
                    << "\tpop {r7, pc}\n\n";

        arm_code = arm_code + return_code.str();
    }
    else
    {
        stringstream return_code;
        return_code << "\tmov sp, r7\n"
                    << "\tadd  sp, sp, #" << sp + 8
                    << "\n"
                    << "\tpop {r7, pc}\n\n";

        arm_code = arm_code + return_code.str();
    }

    //清空语句缓冲区和sp
    sp = 4;
    sub_sp = "";
    getreg_arm_code = "";
}

/*
将变量从内存读出放入寄存器或将寄存器的内容送入内存，由type进行区分 0位读内存 1位写内存
参数为变量名和寄存器编号
*/
string mem_to_reg(int type, string var_name, int reg)
{
    int position;
    stringstream code;
    string op;
    if (type == 0)
        op = "ldr";
    else
        op = "str";

    //判断var_name的类型
    if (var_name.find('[') != var_name.npos)
    { //说明是数组项
        int index = var_name.find("[");
        int index2 = var_name.find("]");
        string name = var_name.substr(1, index - 1);
        string position = var_name.substr(index + 1, index2 - index - 1);
        stringstream pos;
        pos << position.substr(1);
        int posi;
        //判断position是立即数还是临时变量
        if (position[0] == '%')
        {
            code << "\tldr r8, [r7, #" << find_position(position) << "]\n";
            code << "\tmov r5, #4\n";
            code << "\tmul r6, r8, r5"
                 << "\n";
            //判断是全局数组还是局部数组
            if (var_name[0] == '@')
            {
                code << "\tldr r4, =" << name << "\n";
                code << "\tb .LIC" << ik
                     << "\n";
                code << "\t.ltorg"
                     << "\n";
                code << ".LIC" << ik << ":"
                     << "\n";
                ik++;
                code << "\t" << op << " r" << reg << ", [r4, r6"
                     << "]\n";
            }
            else if (var_name[0] == '%')
            {
                code << "\tldr r4, [r7, #" << find_position(name) << "]\n";
                code << "\t" << op << " r" << reg << ", [r4, r6"
                     << "]\n";
            }
        }
        else if (position[0] == '#')
        {
            pos >> posi;

            code << "\tmov r8, #" << posi << "\n";
            code << "\tmov r5, #4\n";
            code << "\tmul r6, r8, r5"
                 << "\n";
            //判断是全局数组还是局部数组
            if (var_name[0] == '@')
            {
                code << "\tldr r4, =" << name << "\n";
                code << "\tb .LIC" << ik
                     << "\n";
                code << "\t.ltorg"
                     << "\n";
                code << ".LIC" << ik << ":"
                     << "\n";
                ik++;
                code << "\t" << op << " r" << reg << ", [r4, r6"
                     << "]\n";
            }
            else if (var_name[0] == '%')
            {
                code << "\tldr r4, [r7, #" << find_position(name) << "]\n";
                code << "\t" << op << " r" << reg << ", [r4, r6"
                     << "]\n";
            }
        }
        else if (position[0] == '@')
        {
            code << "\tldr r8, =" << position.substr(1) << "\n";
            code << "\tb .LIC" << ik
                 << "\n";
            code << "\t.ltorg"
                 << "\n";
            code << ".LIC" << ik << ":"
                 << "\n";
            ik++;
            code << "\tldr r8, [r8]\n";
            code << "\tmov r5, #4\n";
            code << "\tmul r6, r8, r5"
                 << "\n";
            //判断是全局数组还是局部数组
            if (var_name[0] == '@')
            {
                code << "\tldr r4, =" << name << "\n";
                code << "\tb .LIC" << ik
                     << "\n";
                code << "\t.ltorg"
                     << "\n";
                code << ".LIC" << ik << ":"
                     << "\n";
                ik++;
                code << "\t" << op << " r" << reg << ", [r4, r6"
                     << "]\n";
            }
            else if (var_name[0] == '%')
            {
                code << "\tldr r4, [r7, #" << find_position(name) << "]\n";
                code << "\t" << op << " r" << reg << ", [r4, r6"
                     << "]\n";
            }
        }
    }
    else if (var_name[0] == '%')
    {
        code << "\t" << op << " r" << reg << ", [r7, #";
        position = find_position(var_name);
        code << position << "]"
             << "\n";
    }
    else if (var_name[0] == '@')
    {
        code << "\tldr r4, =" << var_name.substr(1) << "\n";
        code << "\tb .LIC" << ik
             << "\n";
        code << "\t.ltorg"
             << "\n";
        code << ".LIC" << ik << ":"
             << "\n";
        ik++;
        code << "\t" << op << " r" << reg << ", [r4]"
             << "\n";
    }
    else if (var_name[0] == '#')
    {
        //判断是否是负数
        bool is_negtive = false;
        int i;
        if (var_name[1] == '-')
        {
            is_negtive = true;
            i = atoi(var_name.substr(2).c_str());
        }
        else
            i = atoi(var_name.substr(1).c_str());
        //判断right_value是否大于16位2进制最大值?
        if (i > 65536)
        {
            code << "\tmovw r" << reg << ", #" << i % 65536 << "\n";
            code << "\tmovt r" << reg << ", " << i / 65536 << "\n";
            if (is_negtive)
            {
                code << "\tmov r5, #0\n"
                     << "\tsub r" << reg << ", r5, r" << reg
                     << "\n";
            }
        }
        else
        {
            code << "\tmov r" << reg << ", " << var_name << "\n";
        }
    }

    return code.str();
}

//查找变量在内存中的位置
int find_position(string var_name)
{
    for (int i = 0; i < reg_table.size(); i++)
    {
        if (var_name == reg_table[i]->var_name)
            return reg_table[i]->stack_pos;
    }
    // printf("var not declared.");
    return -1;
}
