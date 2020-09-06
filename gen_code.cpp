#include "gen_code.h"
#include "symbol.h"
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>

std::string ir_code;
static GlobalSymTable globalSymTable;
static LocalSymTable localSymTable;
static std::vector<std::string> constStringTable;
static int const_str_num = 0;
static int tem_var = 0;

static void add_line(std::string line)
{
    ir_code = ir_code + line + "\n";
}

static int calc_const_expr(struct Node *tree)
{
    if (tree->type == AST_INT_CONST)
        return tree->ival;
    if (tree->type == AST_LVAL_NUM)
    {
        LocalSymbol *local = localSymTable.get(tree->num.name);
        if (local == NULL)
        {
            GlobalSymbol *global = globalSymTable.get(tree->num.name);
            return global->intVal;
        }
        else
        {
            return local->intVal;
        }
    }


    if (tree->type == AST_OP_BINARY)
    {
        switch (tree->binary.op[0])
        {
        case '+':
            return calc_const_expr(tree->binary.left) + calc_const_expr(tree->binary.right);
        case '-':
            return calc_const_expr(tree->binary.left) - calc_const_expr(tree->binary.right);
        case '*':
            return calc_const_expr(tree->binary.left) * calc_const_expr(tree->binary.right);
        case '/':
            return calc_const_expr(tree->binary.left) / calc_const_expr(tree->binary.right);
        case '%':
            return calc_const_expr(tree->binary.left) % calc_const_expr(tree->binary.right);
        default:
            break;
        }
    }
    else if (tree->type == AST_OP_UNARY)
    {
        if (tree->unary.op == '-')
            return -calc_const_expr(tree->unary.operand);
        else
            return calc_const_expr(tree->unary.operand);
    }
    return 0;
}

//参数：数组的定义节点
//若初始化列表为空，返回字符串"{0.<数组长度>}"
//若初始化列表非空，如{{1,2},{3,4}}返回字符串"{1,2,0.16,3,4,0.16,0.72}"
/*static std::string get_array_init_str(Node *tree, LocalSymbol *symbol, const std::string name, int begin, int length, int dim, int depth)
{
    static std::stringstream buffer;

    if (tree->content_for_list == NULL)
    {
        for (int i = begin; i < length * dim; ++i)
        {
            //stmt_buffer << name << "[" << i << "] = #0\n";
        }
    }
    else
    {
        Node *p = tree;
        int i = 0;
        while (p)
        {
            if (p->content_for_list->type != AST_LIST)
            {
                std::string addr = gen_code_expr(stmt_buffer, p->content_for_list);
                stmt_buffer << name << "[#" << i + begin << "] = " << addr << "\n";
                i++;
            }
            else
            {
                while (i % length != 0)
                {
                    //stmt_buffer << name << "[" << i + begin << "] = #0\n";
                    i++;
                }
                gen_code_array_init(p->content_for_list, symbol, name, begin + i,
                                    length / symbol->array_dim[depth + 1], symbol->array_dim[depth + 1],
                                    depth + 1);
                i += length;
            }
            p = p->list;
        }
        while (i < dim * length)
        {
            //stmt_buffer << name << "[" << begin + i << "] = #0\n";
            ++i;
        }
    }
}*/
static std::string get_array_init_str(struct Node *tree)
{
    struct Node *explist = tree->array.expList;
    struct Node *p = tree->array.initval;
    std::stringstream array_value;
    array_value << "{";

    std::vector<int> exp; //用于存储数组每一维的大小
    int size = 1;         //数组总大小
    while (explist)
    {
        //size *= explist->content_for_list->ival;
        //exp.push_back(explist->content_for_list->ival);
        size *= calc_const_expr(explist->content_for_list);
        exp.push_back(calc_const_expr(explist->content_for_list));
        explist = explist->list;
    }

    int i = 0; //i表示数组的初始化值是第几个
    int k = 0; //k表示数组的初始化值在是某一行的第几个
    int zero_num;
    while (p != NULL && p->content_for_list != NULL)
    {
        if (p->content_for_list->type == AST_LIST)
        {
            struct Node *content = p->content_for_list;
            while (content != NULL)
            {
                //array_value << content->content_for_list->ival << ",";
                array_value << calc_const_expr(content->content_for_list) << ",";
                content = content->list;
                k++;
            }
            zero_num = (exp[1] - k) * 4;
            array_value << "0." << zero_num << ",";
            i += exp[1];
            p = p->list;
            k = 0;
        }
        else
        {
            //array_value << p->content_for_list->ival << ",";
            array_value << calc_const_expr(p->content_for_list) << ",";
            p = p->list;
            i++;
        }
    }

    zero_num = (size - i) * 4;
    array_value << "0." << zero_num;
    array_value << "}";
    return array_value.str();
}

static std::string get_next_temp_var()
{

    char s[8];
    sprintf(s, "%%%d", ++tem_var);
    return s;
}

static std::string get_current_temp_var()
{
    char s[8];
    sprintf(s, "%%%d", tem_var);
    return s;
}

static int label_num = 0;
static std::string get_next_label()
{
    return std::string(".L") + std::to_string(++label_num);
}

static void clear_temp_var()
{
    tem_var = 0;
}

static std::string get_symbol_internal_name(const char *org_name)
{
    LocalSymbol *p = localSymTable.get(org_name);
    if (p != NULL)
        return p->internal_name();
    GlobalSymbol *q = globalSymTable.get(org_name);
    if (q != NULL)
        return std::string("@") + org_name;
    return "";
}

static Symbol *get_symbol_from_name(const char *name)
{
    Symbol *symbol = localSymTable.get(name);
    if (symbol == NULL)
        symbol = globalSymTable.get(name);
    return symbol;
}

static void add_external_lib()
{
    GlobalSymbol *symbol;

    globalSymTable.add("getint", SYM_TYPE_FUNC_INT);

    globalSymTable.add("getch", SYM_TYPE_FUNC_INT);

    symbol = globalSymTable.add("getarray", SYM_TYPE_FUNC_INT);
    symbol->func_arg_type_list.push_back(PARAM_ARRAY);

    symbol = globalSymTable.add("putint", SYM_TYPE_FUNC_VOID);
    symbol->func_arg_type_list.push_back(PARAM_INT);

    symbol = globalSymTable.add("putch", SYM_TYPE_FUNC_VOID);
    symbol->func_arg_type_list.push_back(PARAM_INT);

    symbol = globalSymTable.add("putarray", SYM_TYPE_FUNC_VOID);
    symbol->func_arg_type_list.push_back(PARAM_INT);
    symbol->func_arg_type_list.push_back(PARAM_ARRAY);

    globalSymTable.add("starttime", SYM_TYPE_FUNC_VOID);

    globalSymTable.add("stoptime", SYM_TYPE_FUNC_VOID);

    symbol = globalSymTable.add("putf", SYM_TYPE_FUNC_VOID);
    symbol->is_param_fixed = false;
}

void print_ircode(struct Node *tree)
{
    gen_code(tree);
    std::cout << "\n"
              << ir_code << "\n";
}

void gen_code(struct Node *tree)
{
    add_external_lib();

    int i;
    struct Node *p = tree;
    while (p)
    {
        struct Node *content = p->content_for_list;
        int type = content->type;
        switch (type)
        {
        case AST_CONST_DECL: //常量声明or定义
        case AST_VAR_DECL:   //变量声明or定义
            gen_code_global_decl(content);
            break;
        case AST_FUNC_DEF: //函数定义
            gen_code_func_def(content);
            break;
        default:
            break;
        }
        p = p->list;
    }
    std::string const_str_buffer;
    for (int i = 0; i < constStringTable.size(); ++i)
        const_str_buffer += "@.ascii" + std::to_string(i) + " \"" + constStringTable[i] + "\"\n";
    ir_code = const_str_buffer + ir_code;
}

void gen_code_global_decl(struct Node *tree)
{
    struct Node *defList = tree->decl.defList;
    //std::stringstream line;
    while (defList)
    {
        struct Node *content = defList->content_for_list;
        std::string head; //常量or变量前缀
        std::stringstream line;
        switch (content->type) //获取常量or变量前缀
        {
        case AST_VAR_ARRAY_DEF:
        case AST_VAR_NUM_DEF:
            head = "var";
            break;
        case AST_CONST_ARRAY_DEF:
        case AST_CONST_NUM_DEF:
            head = "const";
            break;
        default:
            break;
        }

        switch (content->type) //添加至全局符号表
        {
        case AST_VAR_ARRAY_DEF:
            globalSymTable.add(content->array.name, SYM_TYPE_VAR_ARRAY);
            break;
        case AST_VAR_NUM_DEF:
            globalSymTable.add(content->num.name, SYM_TYPE_VAR_INT);
            break;
        case AST_CONST_ARRAY_DEF:
            globalSymTable.add(content->array.name, SYM_TYPE_CONST_ARRAY);
            break;
        case AST_CONST_NUM_DEF:
            globalSymTable.add(content->num.name, SYM_TYPE_CONST_INT);
            break;
        default:
            break;
        }

        switch (content->type) //生成中间代码
        {
        case AST_VAR_ARRAY_DEF: //数组类型
        case AST_CONST_ARRAY_DEF:
        {
            int length = 1; //数组长度
            struct Node *p = content->array.expList;
            GlobalSymbol *symbol = globalSymTable.get(content->array.name);
            while (p)
            {
                int n = calc_const_expr(p->content_for_list);
                symbol->array_dim.push_back(n);
                length *= n;
                p = p->list;
            }

            if (content->array.initval == NULL)
            {
                line << "array_non_init "
                     << "@" << content->array.name
                     << " = [" << length << "]";
            }
            else
            {
                line << head << "_array_init "
                     << "@" << content->array.name
                     << " = [" << length << "]"
                     << get_array_init_str(content);
            }
            break;
        }
        case AST_VAR_NUM_DEF: //int类型
        case AST_CONST_NUM_DEF:
        {
            if (content->num.initVal == NULL)
            {
                line << "int_non_init "
                     << "@" << content->num.name;
            }
            else
            {

                int num = calc_const_expr(content->num.initVal);
                globalSymTable.setIntVal(content->num.name, num);
                line << head << "_int_init "
                     << "@" << content->num.name
                     << " = " << num;
            }
            break;
        }
        default:
            break;
        }
        add_line(line.str()); //添加至缓冲区
        defList = defList->list;
    }
}

static std::string return_temp_var;
static std::string return_label;
void gen_code_func_def(struct Node *tree)
{
    return_temp_var = get_next_temp_var();
    return_label = get_next_label();
    if (tree->func.ftype == FUNCTYPE_INT)
    {
        globalSymTable.add(tree->func.fname, SYM_TYPE_FUNC_INT);
        return_temp_var = get_next_temp_var();
    }
    else if (tree->func.ftype == FUNCTYPE_VOID)
    {
        globalSymTable.add(tree->func.fname, SYM_TYPE_FUNC_VOID);
        return_temp_var = "";
    }

    localSymTable.pushTable();
    //clear_temp_var();
    gen_code_func_fparam(tree);
    gen_code_func_body(tree);
    localSymTable.popTable();
}

void gen_code_func_fparam(struct Node *tree)
{
    std::stringstream code;
    code << "function " << tree->func.fname << "(";
    struct Node *p = tree->func.args;
    GlobalSymbol *sym_func = globalSymTable.get(tree->func.fname);
    while (p)
    {
        struct Node *arg = p->content_for_list;
        code << "%" << arg->funcArg.name;
        if (arg->funcArg.isArray)
        {
            sym_func->func_arg_type_list.push_back(PARAM_ARRAY);
            LocalSymbol *array_symbol = localSymTable.add(arg->funcArg.name, SYM_TYPE_VAR_ARRAY);
            struct Node *p = arg->funcArg.expList;
            array_symbol->array_dim.push_back(-1);
            while (p)
            {
                int n = calc_const_expr(p->content_for_list);
                array_symbol->array_dim.push_back(n);
                p = p->list;
            }
            code << array_symbol->lable;
            code << "[]";
        }
        else
        {
            sym_func->func_arg_type_list.push_back(PARAM_INT);
            code << localSymTable.add(arg->funcArg.name, SYM_TYPE_VAR_INT)->lable;
        }

        if (p->list != NULL)
        {
            code << ",";
        }
        p = p->list;
    }
    code << "):";
    add_line(code.str());
}

void gen_code_func_body(struct Node *tree)
{
    std::stringstream decl_buffer;
    std::stringstream stmt_buffer;
    gen_code_block(decl_buffer, stmt_buffer, tree->func.body);
    stmt_buffer << "label " << return_label << "\n";
    stmt_buffer << "return " << return_temp_var << "\n";
    add_line(decl_buffer.str());
    add_line(stmt_buffer.str());
}

void gen_code_block(std::stringstream &decl_buffer, std::stringstream &stmt_buffer, Node *body)
{
    localSymTable.pushTable();
    while (body)
    {
        Node *stmt;
        if (body->content_for_list == NULL)
            stmt = body;
        else
            stmt = body->content_for_list;
        switch (stmt->type)
        {
        case AST_CONST_DECL:
        case AST_VAR_DECL:
            gen_code_local_decl(decl_buffer, stmt_buffer, stmt);
            break;

        case AST_ASSIGN_STMT:
            gen_code_assign_expr(stmt_buffer, stmt);
            break;

        case AST_FUNC_CALL:
            gen_code_expr_func_call(stmt_buffer, stmt);
            break;

        case AST_EXP_STMT:
            gen_code_expr(stmt_buffer, stmt);
            break;

        case AST_IF_STMT:
            gen_code_if_stmt(decl_buffer, stmt_buffer, stmt);
            break;

        case AST_WHILE_STMT:
            gen_code_while_stmt(decl_buffer, stmt_buffer, stmt);
            break;

        case AST_BREAK_STMT:
            gen_code_break_stmt(stmt_buffer, stmt);
            break;

        case AST_CONTINUE_STMT:
            gen_code_continue_stmt(stmt_buffer, stmt);
            break;

        case AST_RETURN_STMT:
            gen_code_return_stmt(stmt_buffer, stmt);
            break;

        case AST_LIST:
            gen_code_block(decl_buffer, stmt_buffer, stmt);
            break;

        default:
            break;
        }
        if (body->content_for_list == NULL)
            break;
        else
            body = body->list;
    }
    localSymTable.popTable();
}

void gen_code_array_init(std::stringstream &stmt_buffer, Node *tree, LocalSymbol *symbol, const std::string name, int begin, int length, int dim, int depth)
{
    if (tree->content_for_list == NULL)
    {
        for (int i = begin; i < length * dim; ++i)
        {
            //stmt_buffer << name << "[" << i << "] = #0\n";
        }
    }
    else
    {
        Node *p = tree;
        int i = 0;
        while (p)
        {
            if (p->content_for_list->type != AST_LIST)
            {
                std::string addr = gen_code_expr(stmt_buffer, p->content_for_list);
                stmt_buffer << name << "[#" << i + begin << "] = " << addr << "\n";
                i++;
            }
            else
            {
                while (i % length != 0)
                {
                    //stmt_buffer << name << "[" << i + begin << "] = #0\n";
                    i++;
                }
                gen_code_array_init(stmt_buffer, p->content_for_list, symbol, name, begin + i,
                                    length / symbol->array_dim[depth + 1], symbol->array_dim[depth + 1],
                                    depth + 1);
                i += length;
            }
            p = p->list;
        }
        while (i < dim * length)
        {
            //stmt_buffer << name << "[" << begin + i << "] = #0\n";
            ++i;
        }
    }
}

void gen_code_local_decl(std::stringstream &decl_buffer, std::stringstream &stmt_buffer, Node *tree)
{
    Node *def_list = tree->decl.defList;
    while (def_list)
    {
        Node *content = def_list->content_for_list;
        switch (content->type)
        {
        case AST_CONST_NUM_DEF:
        {
            LocalSymbol *symbol = localSymTable.add(content->num.name, SYM_TYPE_CONST_INT);
            //decl_buffer << symbol->internal_name() << " = alloca(" << symbol->internal_name() << ") #4\n";
            stmt_buffer << symbol->internal_name() << " = alloca(" << symbol->internal_name() << ") #4\n";
            if (content->num.initVal != NULL)
            {
                symbol->intVal = calc_const_expr(content->num.initVal);
                //stmt_buffer << symbol->internal_name() << " = #" << calc_const_expr(content->num.initVal) << "\n";
            }
            break;
        }
        case AST_VAR_NUM_DEF:
        {
            LocalSymbol *symbol = localSymTable.add(content->num.name, SYM_TYPE_VAR_INT);
            //decl_buffer << symbol->internal_name() << " = alloca(" << symbol->internal_name() << ") #4\n";
            stmt_buffer << symbol->internal_name() << " = alloca(" << symbol->internal_name() << ") #4\n";
            if (content->num.initVal != NULL)
            {
                std::string addr = gen_code_expr(stmt_buffer, content->num.initVal);
                //stmt_buffer << symbol->internal_name() << " = " << get_current_temp_var() << "\n";
                stmt_buffer << symbol->internal_name() << " = " << addr << "\n";
            }
            break;
        }

        case AST_CONST_ARRAY_DEF:
        case AST_VAR_ARRAY_DEF:
        {
            LocalSymbol *symbol = localSymTable.add(content->num.name, SYM_TYPE_VAR_ARRAY);
            int length = 1;
            struct Node *p = content->array.expList;
            while (p)
            {
                int n = calc_const_expr(p->content_for_list);
                symbol->array_dim.push_back(n);
                length *= n;
                p = p->list;
            }
            decl_buffer << symbol->internal_name() << " = alloca #" << length * 4 << "\n";
            //stmt_buffer << symbol->internal_name() << " = alloca #" << length * 4 << "\n";
            if (content->array.initval != NULL)
            {
                std::string addr = get_next_temp_var();
                stmt_buffer << addr << " = & " << symbol->internal_name() << "\n";
                stmt_buffer << "call memset(" << addr << ",#0,#" << length * 4 << ")\n";
                gen_code_array_init(stmt_buffer, content->array.initval, symbol, symbol->internal_name(), 0, length / symbol->array_dim[0], symbol->array_dim[0], 0);
            }
            else
            {
                std::string addr = get_next_temp_var();
                stmt_buffer << addr << " = & " << symbol->internal_name() << "\n";
                stmt_buffer << "call memset(" << addr << ",#0,#" << length * 4 << ")\n";
            }

            break;
        }

        default:
            break;
        }
        def_list = def_list->list;
    }
}

std::string gen_code_expr(std::stringstream &stmt_buffer, Node *tree)
{
    std::string l_addr, r_addr;
    switch (tree->type)
    {
    case AST_INT_CONST:
    {
        l_addr = get_next_temp_var();
        stmt_buffer << l_addr << " = #" << tree->ival << "\n";
        return l_addr;
    }
    case AST_LVAL_NUM:
    {
        Symbol *symbol = get_symbol_from_name(tree->num.name);
        if (symbol->type == SYM_TYPE_CONST_INT)
            return std::string("#") + std::to_string(symbol->intVal);
        return get_symbol_internal_name(tree->num.name);
    }
    case AST_LVAL_ARRAY:
        return gen_code_expr_array(stmt_buffer, tree);
    case AST_FUNC_CALL:
        return gen_code_expr_func_call(stmt_buffer, tree);
    }

    if (tree->type == AST_OP_BINARY)
    {
        l_addr = gen_code_expr(stmt_buffer, tree->binary.left);
        r_addr = gen_code_expr(stmt_buffer, tree->binary.right);
        if (tree->binary.op[0] == '%')
        {
            stmt_buffer << get_next_temp_var()
                        << " = " << l_addr
                        << " "
                        << "mod"
                        << " "
                        << r_addr << "\n";
        }
        else
        {
            stmt_buffer << get_next_temp_var()
                        << " = " << l_addr
                        << " " << tree->binary.op << " "
                        << r_addr << "\n";
        }
    }
    else if (tree->type == AST_OP_UNARY)
    {
        l_addr = gen_code_expr(stmt_buffer, tree->unary.operand);
        stmt_buffer << get_next_temp_var()
                    << " = " << tree->unary.op << " "
                    << l_addr << "\n";
    }

    return get_current_temp_var();
}

void gen_code_assign_expr(std::stringstream &stmt_buffer, Node *tree)
{
    std::string l_addr = gen_code_lval(stmt_buffer, tree->binary.left);
    std::string r_addr = gen_code_expr(stmt_buffer, tree->binary.right);
    stmt_buffer << l_addr << " = " << r_addr << "\n";
}

static Node *get_list_content_n(Node *tree, int n)
{
    int i = 0;
    while (tree && i < n)
    {
        if (tree->list == NULL)
            return tree->content_for_list;
        ++i;
        tree = tree->list;
    }
    return tree->content_for_list;
}

std::string gen_code_lval(std::stringstream &stmt_buffer, Node *tree)
{
    switch (tree->type)
    {
    case AST_LVAL_NUM:
        return get_symbol_internal_name(tree->num.name);

    case AST_LVAL_ARRAY: //a[b+c];int a[f][g];
    {
        Node *p = tree->array.expList;
        std::stringstream result;
        Symbol *symbol = localSymTable.get(tree->array.name);
        if (symbol == NULL)
            symbol = globalSymTable.get(tree->array.name);

        std::string addr1, addr2, sum = "#0";
        int dim = symbol->array_dim.size();
        int offset = 1;
        for (int i = 0; i < dim; i++)
        {
            addr1 = gen_code_expr(stmt_buffer, get_list_content_n(p, dim - i - 1));
            addr2 = get_next_temp_var();

            stmt_buffer << addr2
                        << " = " << addr1
                        << " * #" << offset << "\n";

            offset *= symbol->array_dim[dim - i - 1];

            stmt_buffer << get_next_temp_var()
                        << " = " << addr2
                        << " + " << sum << "\n";
            sum = get_current_temp_var();
        }
        result << get_symbol_internal_name(tree->array.name) << "[" << sum << "]";
        return result.str();
    }

    default:
        return "";
    }
}

std::string gen_code_expr_array(std::stringstream &stmt_buffer, Node *tree)
{
    std::string addr1 = get_next_temp_var();
    std::string addr2 = gen_code_lval(stmt_buffer, tree);
    stmt_buffer << addr1 << " = " << addr2 << "\n";
    return addr1;
}

std::string gen_code_expr_func_call(std::stringstream &stmt_buffer, Node *tree)
{
    Node *list = tree->func.args;
    std::vector<std::string> args = gen_code_func_Rparams(stmt_buffer, tree);

    GlobalSymbol *func = globalSymTable.get(tree->func.fname);

    if (func->type == SYM_TYPE_FUNC_INT)
    {
        stmt_buffer << get_next_temp_var() << " = ";
    }

    if (!strcmp(tree->func.fname, "putf"))
        stmt_buffer << "call printf(";
    else
        stmt_buffer << "call " << tree->func.fname << "(";

    int size = args.size();
    for (int i = 0; i < size; ++i)
    {
        stmt_buffer << args[i];
        if (i < size - 1)
        {
            stmt_buffer << ",";
        }
    }

    stmt_buffer << ")\n";
    return get_current_temp_var();
}

std::vector<std::string> gen_code_func_Rparams(std::stringstream &stmt_buffer, Node *tree)
{
    GlobalSymbol *sym_func = globalSymTable.get(tree->func.fname);
    std::vector<std::string> args;
    std::string addr;
    Node *p = tree->func.args;
    if (sym_func->is_param_fixed)
    {
        for (int param_type : sym_func->func_arg_type_list)
        {
            switch (param_type)
            {
            case PARAM_INT:
                addr = gen_code_expr(stmt_buffer, p->content_for_list);
                break;
            case PARAM_ARRAY:
                addr = gen_code_func_Rparams_array(stmt_buffer, p->content_for_list);
                break;
            default:
                break;
            }
            args.push_back(addr);
            p = p->list;
        }
    }
    else
    {
        while (p)
        {
            if (p->content_for_list->type == AST_STR_CONST)
            {
                constStringTable.push_back(p->content_for_list->sval);
                addr = std::string("@.ascii") + std::to_string(const_str_num++);
            }
            else
            {
                addr = gen_code_expr(stmt_buffer, p->content_for_list);
            }
            args.push_back(addr);
            p = p->list;
        }
    }

    return args;
}

static int get_array_rparam_dim(Node *tree)
{
    Node *p = tree->array.expList;
    int n = 0;
    while (p)
    {
        n++;
        p = p->list;
    }
    return n;
}

std::string gen_code_func_Rparams_array(std::stringstream &stmt_buffer, Node *tree)
{
    Symbol *symbol = localSymTable.get(tree->array.name);
    if (symbol == NULL)
        symbol = globalSymTable.get(tree->array.name);

    Node *p = tree->array.expList;
    std::string addr1, addr2, sum = "#0";
    int def_dim = symbol->array_dim.size();
    int rparam_dim = get_array_rparam_dim(tree);
    int offset = 1;
    for (int i = 0; i < def_dim - rparam_dim; ++i)
        offset *= symbol->array_dim[def_dim - i - 1];

    for (int i = 0; i < rparam_dim; i++)
    {
        addr1 = gen_code_expr(stmt_buffer, get_list_content_n(p, rparam_dim - i - 1));
        addr2 = get_next_temp_var();

        stmt_buffer << addr2
                    << " = " << addr1
                    << " * #" << offset << "\n";

        offset *= symbol->array_dim[rparam_dim - i - 1];

        stmt_buffer << get_next_temp_var()
                    << " = " << addr2
                    << " + " << sum << "\n";
        sum = get_current_temp_var();
    }

    std::string addr_offset = get_next_temp_var();
    stmt_buffer << addr_offset << " = " << sum << " * #4\n";

    addr1 = get_next_temp_var();
    stmt_buffer << addr1 << " = & " << get_symbol_internal_name(tree->array.name) << "\n";
    //stmt_buffer << get_next_temp_var() << " = " << addr1 << " + " << sum << "\n";
    stmt_buffer << get_next_temp_var() << " = " << addr1 << " + " << addr_offset << "\n";

    return get_current_temp_var();
}

static const std::string FALL = ".fall";

void gen_code_if_stmt(std::stringstream &decl_buffer, std::stringstream &stmt_buffer, Node *tree)
{
    std::string true_label = get_next_label();
    std::string false_label = get_next_label();
    std::string end_label = get_next_label();

    if (tree->ifStmt.els != NULL)
    {
        gen_code_cond(stmt_buffer, tree->ifStmt.cond, FALL, false_label);

        gen_code_block(decl_buffer, stmt_buffer, tree->ifStmt.then);
        stmt_buffer << "goto " << end_label << "\n";

        stmt_buffer << "\nlabel " << false_label << "\n";
        gen_code_block(decl_buffer, stmt_buffer, tree->ifStmt.els);
    }
    else
    {
        gen_code_cond(stmt_buffer, tree->ifStmt.cond, FALL, end_label);
        gen_code_block(decl_buffer, stmt_buffer, tree->ifStmt.then);
    }

    stmt_buffer << "\nlabel " << end_label << "\n";
}

std::vector<std::string> while_begin_label;
std::vector<std::string> while_end_label;

void gen_code_while_stmt(std::stringstream &decl_buffer, std::stringstream &stmt_buffer, Node *tree)
{
    std::string begin_label = get_next_label();
    std::string true_label = get_next_label();
    std::string false_label = get_next_label();

    while_begin_label.push_back(begin_label);
    while_end_label.push_back(false_label);

    stmt_buffer << "\nlabel " << begin_label << "\n";
    gen_code_cond(stmt_buffer, tree->whileStmt.cond, FALL, false_label);
    gen_code_block(decl_buffer, stmt_buffer, tree->whileStmt.block);
    stmt_buffer << "goto " << begin_label << "\n";
    stmt_buffer << "\nlabel " << false_label << "\n";

    while_begin_label.pop_back();
    while_end_label.pop_back();
}

void gen_code_cond(std::stringstream &stmt_buffer, Node *cond, const std::string &true_label, const std::string &false_label)
{
    if (cond->type == AST_OP_BINARY && !strcmp(cond->binary.op, "||"))
    {
        std::string l_true = (true_label != FALL) ? true_label : get_next_label();
        std::string l_false = FALL;

        gen_code_cond(stmt_buffer, cond->binary.left, l_true, l_false);
        gen_code_cond(stmt_buffer, cond->binary.right, true_label, false_label);

        if (true_label == FALL)
            stmt_buffer << "label " << l_true << "\n";
    }
    else if (cond->type == AST_OP_BINARY && !strcmp(cond->binary.op, "&&"))
    {
        std::string l_true = FALL;
        std::string l_false = (false_label != FALL) ? false_label : get_next_label();
        gen_code_cond(stmt_buffer, cond->binary.left, l_true, l_false);
        gen_code_cond(stmt_buffer, cond->binary.right, true_label, false_label);
        if (false_label == FALL)
            stmt_buffer << "label " << l_false << "\n";
    }
    else if (cond->type == AST_OP_UNARY && cond->unary.op == '!')
    {
        gen_code_cond(stmt_buffer, cond->unary.operand, false_label, true_label);
    }
    else
    {
        gen_code_rel_expr(stmt_buffer, cond, true_label, false_label);
    }
}

static bool is_cmp_rel_expr(Node *expr)
{
    if (expr->type == AST_OP_BINARY)
    {
        if (!strcmp(expr->binary.op, "==") || !strcmp(expr->binary.op, "!=") || !strcmp(expr->binary.op, "<=") || !strcmp(expr->binary.op, ">=") || !strcmp(expr->binary.op, ">") || !strcmp(expr->binary.op, "<"))
        {
            return true;
        }
    }
    return false;
}

static std::string get_cmp_opposite(const char *op)
{
    if (!strcmp(op, "=="))
        return "!=";

    if (!strcmp(op, "!="))
        return "==";

    if (!strcmp(op, ">="))
        return "<";

    if (!strcmp(op, "<="))
        return ">";

    if (!strcmp(op, ">"))
        return "<=";

    if (!strcmp(op, "<"))
        return ">=";

    return "";
}

void gen_code_rel_expr(std::stringstream &stmt_buffer, Node *expr, const std::string &true_label, const std::string &false_label)
{
    std::string op;
    std::string l_addr;
    std::string r_addr;
    if (is_cmp_rel_expr(expr))
    {
        op = expr->binary.op;
        l_addr = gen_code_expr(stmt_buffer, expr->binary.left);
        r_addr = gen_code_expr(stmt_buffer, expr->binary.right);
    }
    else
    {
        op = "!=";
        l_addr = gen_code_expr(stmt_buffer, expr);
        r_addr = "#0";
    }

    if (true_label != FALL && false_label != FALL)
    {
        stmt_buffer << "if "
                    << l_addr << " " << op << " " << r_addr
                    << " goto " << true_label << "\n";
        stmt_buffer << "goto " << false_label << "\n\n";
    }
    else if (true_label != FALL)
    {
        stmt_buffer << "if "
                    << l_addr << " " << op << " " << r_addr
                    << " goto " << true_label << "\n\n";
    }
    else if (false_label != FALL)
    {
        stmt_buffer << "if "
                    << l_addr << " " << get_cmp_opposite(op.c_str()) << " " << r_addr
                    << " goto " << false_label << "\n\n";
    }
}

void gen_code_break_stmt(std::stringstream &stmt_buffer, Node *tree)
{
    stmt_buffer << "goto " << while_end_label.back() << "\n";
}

void gen_code_continue_stmt(std::stringstream &stmt_buffer, Node *tree)
{
    stmt_buffer << "goto " << while_begin_label.back() << "\n";
}

void gen_code_return_stmt(std::stringstream &stmt_buffer, Node *tree)
{
    if (tree->retval != NULL)
    {
        std::string addr = gen_code_expr(stmt_buffer, tree->retval);
        stmt_buffer << return_temp_var << " = " << addr << "\n";
    }
    stmt_buffer << "goto " << return_label << "\n";
}