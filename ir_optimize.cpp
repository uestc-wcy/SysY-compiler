#include "ir_optimize.h"
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

extern std::string ir_code;
std::string optimized_ir_code;
std::list<IrFunction *> ir_functions;
std::vector<std::string> global_decl_stmt;
std::vector<std::string> reducible_func_list;
int IrFunction::label_num = 0;
//int IrExpr::sum = 0;

std::vector<std::string> str_split(const std::string &s, const std::string &c)
{
    std::vector<std::string> v;
    std::string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while (std::string::npos != pos2)
    {
        v.push_back(s.substr(pos1, pos2 - pos1));

        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if (pos1 != s.length())
        v.push_back(s.substr(pos1));

    return v;
}

bool find(const std::string &s, const std::string &c)
{
    if (s.find(c) != s.npos)
        return true;
    return false;
}

int type(const std::string &var)
{
    switch (var[0])
    {
    case '%':
        if (isdigit(var[1]))
            return VAR_TYPE_TEMP;
        else
            return VAR_TYPE_LOCAL;

    case '@':
        return VAR_TYPE_GLOBAL;

    case '#':
        return VAR_TYPE_NUM;

    default:
        return VAR_TYPE_NONE;
    }
}

int var_type(const std::string &var)
{
    return type(var);
}

std::string get_array_index(std::string &target)
{
    return std::string(target.begin() + target.find("[") + 1, target.end() - 1);
}

std::string get_array_name(std::string &target)
{
    if (!find(target, "["))
        return target;
    return std::string(target.begin(), target.begin() + target.find("["));
}

static void build_all_basic_blocks()
{
    char *p = strdup(ir_code.c_str());
    char *s = strtok(p, "\n");
    std::stringstream decl_buffer;
    while (s)
    {
        if (str_split(s, " ")[0] == "function")
        {
            IrFunction *function = new IrFunction();
            function->add(s);
            do
            {
                s = strtok(NULL, "\n");
                if (strcmp(s, "")) //skip the blank line
                    function->add(s);
            } while (s && str_split(s, " ")[0] != "return");
            function->build_all_basic_block();
            ir_functions.push_back(function);
        }
        else
        {
            decl_buffer << s << "\n";
            global_decl_stmt.push_back(std::string(s) + "\n");
        }
        s = strtok(NULL, "\n");
    }
    optimized_ir_code += decl_buffer.str();
    free(p);
}

static void to_string()
{
    for (auto ir_func : ir_functions)
        optimized_ir_code += ir_func->to_string();
}

static void print_optimized_ir()
{
    std::cout << optimized_ir_code;
}

void find_reducible_func()
{
    reducible_func_list.push_back("memset");
    for (auto function : ir_functions)
    {
        if (function->is_reducible())
        {
            reducible_func_list.push_back(function->get_name());
        }
    }
}


void ir_optimize()
{
    build_all_basic_blocks();
    find_reducible_func();

    for (auto function : ir_functions)
    {
        function->active_var_analyse();
        function->optimize_basic_blocks();
        function->global_optimize();
        //function->print_active_var();
        //function->print_usable_expr();
    }

    to_string();
    print_optimized_ir();
}