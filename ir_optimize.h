#ifndef IR_OPTIMIZE_H
#define IR_OPTIMIZE_H
#include <string.h>
#include <stdlib.h>
#include <list>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <set>
#include <sstream>
#include <iostream>

std::vector<std::string> str_split(const std::string &s, const std::string &seperator);

bool find(const std::string &s, const std::string &c);

std::string get_array_index(std::string &target);

std::string get_array_name(std::string &target);

template <class Tk, class Tp>
std::vector<Tk> get_keys_from_value(const std::map<Tk, Tp> &map, const Tp &e)
{
    std::vector<Tk> keys;
    for (auto p = map.begin(); p != map.end(); ++p)
    {
        if (p->second == e)
        {
            keys.push_back(p->first);
        }
    }
    return keys;
}

template <class Tk, class Tp>
std::vector<Tk> get_keys(const std::map<Tk, Tp> &map)
{
    std::vector<Tk> keys;
    for (auto p = map.begin(); p != map.end(); ++p)
        keys.push_back(p->first);

    return keys;
}

template <class T>
bool is_child(const std::set<T> &a, const std::set<T> &b) //b是否为a的子集
{
    for (T e : b)
        if (a.find(e) == a.end())
            return false;

    return true;
}

template <class T>
bool find(const std::vector<T> &vec, const T &a)
{
    for (auto e : vec)
        if (e == a)
            return true;

    return false;
}

template <class T>
bool find(const std::set<T> &set, const T &a)
{
    return set.find(a) != set.end();
}

template <class T>
bool find(const std::list<T> &list, const T &a)
{
    for (auto e : list)
        if (e == a)
            return true;

    return false;
}

template <class T>
bool find(const T *array, const T a, int n)
{
    for (int i = 0; i < n; ++i)
    {
        if (array[i] == a)
        {
            return true;
        }
    }
    return false;
}

template <class T>
void erase(std::vector<T> &vec, const T &a)
{
    for (int i = 0; i < vec.size(); ++i)
    {
        if (vec[i] == a)
        {
            vec.erase(vec.begin() + i);
            break;
        }
    }
}

template <class T>
std::vector<T> to_array(const std::set<T> &set)
{
    std::vector<T> vec;
    for (auto e : set)
        vec.push_back(e);
    return vec;
}

template <class T>
bool equal(const std::set<T> &set1, const std::set<T> &set2)
{
    if (set1.size() != set2.size())
        return false;
    for (auto &e : set1)
    {
        if (set2.find(e) == set2.end())
        {
            return false;
        }
    }
    return true;
}

template <class T>
bool insert(std::vector<T> &vec, int index, const T &e)
{
    if (index > vec.size() || index < 0)
        return false;

    vec.push_back(vec[vec.size() - 1]);
    for (int i = vec.size() - 1; i > index; --i)
        vec[i] = vec[i - 1];

    vec[index] = e;
    return true;
}

extern std::vector<std::string> reducible_func_list;

enum
{
    VAR_TYPE_TEMP,
    VAR_TYPE_LOCAL,
    VAR_TYPE_GLOBAL,
    VAR_TYPE_NUM,
    VAR_TYPE_NONE
};

int type(const std::string &var);
int var_type(const std::string &var);

enum
{
    IR_TYPE_GOTO,
    IR_TYPE_IF,
    IR_TYPE_BINARY,
    IR_TYPE_UNARY,
    IR_TYPE_ASSIGN,
    IR_TYPE_RETURN,
    IR_TYPE_FUNC_CALL,
    IR_TYPE_OTHER
};

enum
{
    IR_EXPR_BINARY,
    IR_EXPR_UNARY,
    IR_EXPR_COPY,
    IR_EXPR_ARRAY
};
struct IrStmt;
struct IrExpr
{
    int type;
    std::string l_addr;
    std::string r_addr;
    std::string op;
    std::string target;

    IrStmt *stmt;

    bool operator<(const IrExpr expr) const
    {
        std::string s1 = target + l_addr + op + r_addr;
        std::string s2 = expr.target + expr.l_addr + expr.op + expr.r_addr;
        return s1 < s2;
    }

    bool operator==(const IrExpr &expr) const
    {
        if (type == IR_EXPR_COPY && expr.type == IR_EXPR_COPY)
        {
            if (r_addr == expr.r_addr && target == expr.target)
                return true;
        }
        else if (l_addr == expr.l_addr && r_addr == expr.r_addr && op == expr.op)
            return true;

        return false;
    }
};

class BasicBlock;

struct Definition
{
    IrStmt *stmt;
    BasicBlock *block;
    std::string def_var;

    bool operator<(const Definition &definition) const
    {
        return stmt < definition.stmt;
    }

    bool operator==(const Definition &definition) const
    {
        return stmt == definition.stmt && block == definition.block;
    }

    bool operator!=(const Definition &definition) const
    {
        return !operator==(definition);
    }
};

struct IrStmt
{
    int type;
    int num;

    std::string l_addr;
    std::string r_addr;
    std::string op;
    std::string target;
    std::string func_name;
    std::vector<std::string> func_args;
    std::vector<Definition> use_def;
    std::vector<std::string> use_var_list;
    std::string other_info;

    std::string to_string()
    {
        switch (type)
        {
        case IR_TYPE_ASSIGN:
            return target + " = " + r_addr;

        case IR_TYPE_BINARY:
            return target + " = " + l_addr + " " + op + " " + r_addr;

        case IR_TYPE_GOTO:
            return std::string("goto ") + target;

        case IR_TYPE_IF:
            return std::string("if ") + l_addr + " " + op + " " + r_addr + " goto " + target;

        case IR_TYPE_UNARY:
            return target + " = " + op + " " + r_addr;

        case IR_TYPE_FUNC_CALL:
        {
            std::string decl = std::string("call ") + func_name + "(";
            for (int i = 0; i < func_args.size(); ++i)
            {
                decl += func_args[i];
                if (i != func_args.size() - 1)
                    decl += ",";
            }
            decl += ")";
            if (target == "")
                return decl;
            else
                return target + " = " + decl;
        }

        case IR_TYPE_RETURN:
            return std::string("return ") + target;

        case IR_TYPE_OTHER:
            return other_info;

        default:
            return "";
        }
    }

    bool get_expr(IrExpr *ir_expr)
    {
        switch (type)
        {
        case IR_TYPE_BINARY:
        {
            ir_expr->l_addr = l_addr;
            ir_expr->r_addr = r_addr;
            ir_expr->op = op;
            ir_expr->target = target;
            ir_expr->type = IR_EXPR_BINARY;
            ir_expr->stmt = this;
            return true;
        }

        case IR_TYPE_UNARY:
        {
            ir_expr->r_addr = r_addr;
            ir_expr->op = op;
            ir_expr->target = target;
            ir_expr->type = IR_EXPR_UNARY;
            ir_expr->stmt = this;
            return true;
        }

        case IR_TYPE_ASSIGN:
        {
            if (target.find('[') != target.npos)
                return false;
            if (r_addr.find('[') == r_addr.npos) //copy
            {
                ir_expr->r_addr = r_addr;
                ir_expr->target = target;
                ir_expr->type = IR_EXPR_COPY;
                ir_expr->stmt = this;
                return true;
            }
            else //array
            {
                ir_expr->l_addr = std::string(r_addr.begin(), r_addr.begin() + r_addr.find("["));
                ir_expr->r_addr = std::string(r_addr.begin() + r_addr.find("[") + 1, r_addr.end() - 1);
                ir_expr->target = target;
                ir_expr->type = IR_EXPR_ARRAY;
                ir_expr->stmt = this;
                return true;
            }
        }
        default:
            return false;
        }
    }

    bool exist_use(const std::string &var)
    {
        if (var == l_addr || var == r_addr)
            return true;

        if (find(target, "["))
            if (get_array_index(target) == var)
                return true;

        if (find(r_addr, "["))
            if (get_array_index(r_addr) == var || get_array_name(r_addr) == var)
                return true;

        if (type == IR_TYPE_RETURN && target == var)
            return true;

        for (std::string arg : func_args)
            if (arg == var)
                return true;

        return false;
    }

    std::vector<std::string> get_use_list()
    {
        std::vector<std::string> vec;
        if (l_addr != "" && var_type(l_addr) != VAR_TYPE_NUM)
            vec.push_back(l_addr);

        if (r_addr != "")
        {
            if (find(r_addr, "["))
            {
                vec.push_back(get_array_index(r_addr));
                vec.push_back(get_array_name(r_addr));
            }
            else if (var_type(r_addr) != VAR_TYPE_NUM)
            {
                vec.push_back(r_addr);
            }
        }

        if (find(target, "["))
            vec.push_back(get_array_index(target));

        if (type == IR_TYPE_RETURN && target != "")
            vec.push_back(target);

        for (std::string arg : func_args)
            vec.push_back(arg);

        return vec;
    }

    std::vector<Definition> get_def(const std::string &var) //if exist var use
    {
        std::vector<Definition> vec;
        for (auto def : use_def)
        {
            if (def.def_var == var)
            {
                vec.push_back(def);
            }
        }
        return vec;
    }

    bool has_definition()
    {
        if (type == IR_TYPE_GOTO || type == IR_TYPE_IF || type == IR_TYPE_RETURN || target == "")
        {
            return false;
        }
        return true;
    }

    std::string get_def_var()
    {
        if (!has_definition())
            return "";
        return get_array_name(target);
    }

    IrStmt() {}
    ~IrStmt() {}
};

class BasicBlock
{
public:
    std::list<BasicBlock *> predecessor;
    std::list<BasicBlock *> successor;
    std::vector<IrStmt *> stmts;

    std::set<std::string> active_var_def;
    std::set<std::string> active_var_use;
    std::set<std::string> active_var_in;
    std::set<std::string> active_var_out;

    std::set<IrExpr> expr_gen;
    std::set<IrExpr> expr_kill;
    std::set<IrExpr> expr_in;
    std::set<IrExpr> expr_out;

    std::set<Definition> def_gen;
    std::set<Definition> def_kill;
    std::set<Definition> def_in;
    std::set<Definition> def_out;

    std::set<BasicBlock *> dom_node_in;
    std::set<BasicBlock *> dom_node_out;

    std::string label;
    bool has_label;

    void add(std::string ir)
    {
        IrStmt *ir_stmt = new IrStmt();
        std::vector<std::string> parts = str_split(ir, " ");
        if (parts[0] == "if")
        {
            ir_stmt->type = IR_TYPE_IF;
            ir_stmt->l_addr = parts[1];
            ir_stmt->op = parts[2];
            ir_stmt->r_addr = parts[3];
        }
        else if (parts[0] == "goto")
        {
            ir_stmt->type = IR_TYPE_GOTO;
        }
        else if (parts[0] == "call")
        {
            ir_stmt->type = IR_TYPE_FUNC_CALL;
            std::string::size_type left_parentheses = parts[1].find('(');
            std::string::size_type right_parentheses = parts[1].rfind(')');

            ir_stmt->func_name = std::string(parts[1].begin(), parts[1].begin() + left_parentheses);

            std::string args(parts[1].begin() + left_parentheses + 1, parts[1].begin() + right_parentheses);
            ir_stmt->func_args = str_split(args, ",");
        }
        else if (parts[0] == "return")
        {
            ir_stmt->type = IR_TYPE_RETURN;
            ir_stmt->target = parts.size() == 1 ? "" : parts[1];
        }
        else if (parts[1] == "=")
        {
            if (parts[2].find("alloca") != parts[2].npos)
            {
                ir_stmt->type = IR_TYPE_OTHER;
                ir_stmt->other_info = ir;
            }
            else if (parts[2] == "call")
            {
                ir_stmt->type = IR_TYPE_FUNC_CALL;
                std::string::size_type left_parentheses = parts[3].find('(');
                std::string::size_type right_parentheses = parts[3].rfind(')');

                ir_stmt->func_name = std::string(parts[3].begin(), parts[3].begin() + left_parentheses);

                std::string args(parts[3].begin() + left_parentheses + 1, parts[3].begin() + right_parentheses);
                ir_stmt->func_args = str_split(args, ",");
                ir_stmt->target = parts[0];
            }
            else if (parts[2] == "&" || parts[2] == "-" || parts[2] == "+")
            {
                ir_stmt->type = IR_TYPE_UNARY;
                ir_stmt->target = parts[0];
                ir_stmt->r_addr = parts[3];
                ir_stmt->op = parts[2];
            }
            else if (parts.size() == 3)
            {
                ir_stmt->type = IR_TYPE_ASSIGN;
                ir_stmt->target = parts[0];
                ir_stmt->r_addr = parts[2];
                ir_stmt->op = parts[1];
            }
            else
            {
                ir_stmt->type = IR_TYPE_BINARY;
                ir_stmt->target = parts[0];
                ir_stmt->l_addr = parts[2];
                ir_stmt->r_addr = parts[4];
                ir_stmt->op = parts[3];
            }
        }
        stmts.push_back(ir_stmt);
    }

    IrStmt *get_last_stmt()
    {
        if (stmts.size() == 0)
            return NULL;
        for (int i = stmts.size() - 1; i >= 0; --i)
            if (stmts[i] != NULL)
                return stmts[i];
        return NULL;
    }

    void print_def_gen()
    {
        for (auto def : def_gen)
        {
            std::cout << def.stmt->to_string() << ' ';
        }
        std::cout << '\n';
    }

    void print_def_kill()
    {
        for (auto def : def_kill)
        {
            std::cout << def.stmt->to_string() << ' ';
        }
        std::cout << '\n';
    }

    void print_def_in()
    {
        for (auto def : def_in)
        {
            std::cout << def.stmt->to_string() << ' ';
        }
        std::cout << '\n';
    }

    void print_def_out()
    {
        for (auto def : def_out)
        {
            std::cout << def.stmt->to_string() << ' ';
        }
        std::cout << '\n';
    }

    void clear()
    {
        active_var_def.clear();
        active_var_use.clear();
        active_var_in.clear();
        active_var_out.clear();
        expr_gen.clear();
        expr_in.clear();
        expr_kill.clear();
        expr_out.clear();
        def_gen.clear();
        def_in.clear();
        def_kill.clear();
        def_out.clear();
        dom_node_in.clear();
        dom_node_out.clear();
        graph.clear();
        internal_nodes.clear();
        leave_nodes.clear();
        tmp_stmts.clear();
        tmp_stmts_used.clear();

        for (int i = 0; i < stmts.size(); i++)
        {
            if (stmts[i] == NULL)
            {
                for (int j = i; j < stmts.size(); j++)
                {
                    stmts[j] = stmts[j + 1];
                }
                stmts.pop_back();
                i--;
            }
        }
    }

    std::vector<std::string> get_use_list()
    {
        std::vector<std::string> vec;
        for (IrStmt *stmt : stmts)
        {
            if (stmt == NULL)
                continue;
            for (auto var : stmt->get_use_list())
                vec.push_back(var);
        }
        return vec;
    }

    void add_to_def(const std::string &var)
    {
        if (type(var) == VAR_TYPE_TEMP || type(var) == VAR_TYPE_LOCAL)
        {
            active_var_def.insert(var);
        }
        else if (type(var) == VAR_TYPE_GLOBAL)
        {
            active_var_use.insert(var);
            active_var_out.insert(var);
        }
    }

    void add_to_use(const std::string &var)
    {
        if (type(var) != VAR_TYPE_NUM && type(var) != VAR_TYPE_NONE &&
            active_var_def.find(var) == active_var_def.end())
        {
            active_var_use.insert(var);
        }
        if (type(var) == VAR_TYPE_GLOBAL)
        {
            active_var_out.insert(var);
        }
    }

    void find_active_var_def_use()
    {
        for (IrStmt *stmt : stmts)
        {
            if (stmt->target.find("[") != stmt->target.npos)
            {
                std::string var1 = std::string(stmt->target.begin(),
                                               stmt->target.begin() + stmt->target.find("["));
                std::string var2 = std::string(stmt->target.begin() + stmt->target.find("[") + 1,
                                               stmt->target.end() - 1);
                std::string var3 = stmt->r_addr;

                add_to_use(var2);
                add_to_use(var3);
                if (active_var_use.find(var1) == active_var_use.end())
                    add_to_def(var1);
                continue;
            }
            else if (stmt->r_addr.find('[') != stmt->r_addr.npos)
            {
                std::string l_var = std::string(stmt->r_addr.begin(),
                                                stmt->r_addr.begin() + stmt->r_addr.find("["));
                std::string r_var = std::string(stmt->r_addr.begin() + stmt->r_addr.find("[") + 1,
                                                stmt->r_addr.end() - 1);
                add_to_use(l_var);
                add_to_use(r_var);
                if (active_var_use.find(stmt->target) == active_var_use.end())
                    add_to_def(stmt->target);
                continue;
            }
            add_to_use(stmt->l_addr);
            add_to_use(stmt->r_addr);
            if (stmt->type == IR_TYPE_FUNC_CALL)
                for (std::string param : stmt->func_args)
                    add_to_use(param);

            if (stmt->type == IR_TYPE_RETURN && stmt->target != "")
                add_to_use(stmt->target);

            if (active_var_use.find(stmt->target) == active_var_use.end())
                add_to_def(stmt->target);
        }
    }

    void kill_expr(const std::string &target, std::set<IrExpr> &U_expr_set)
    {
        std::set<IrExpr> tmp1;
        for (auto iter = expr_gen.begin(); iter != expr_gen.end(); ++iter)
        {
            if (iter->type == IR_EXPR_ARRAY)
            {
                if (target.find('[') != target.npos)
                {
                    std::string name = std::string(target.begin(), target.begin() + target.find("["));
                    if (name != iter->l_addr)
                        tmp1.insert(*iter);
                }
                else if (target != iter->r_addr)
                {
                    tmp1.insert(*iter);
                }
            }
            else if (iter->type == IR_EXPR_COPY)
            {
                if (iter->target != target && iter->r_addr != target)
                    tmp1.insert(*iter);
                /*if (iter->target == target && iter->r_addr == target)
                    tmp1.insert(*iter);*/
            }
            else if (iter->l_addr != target && iter->r_addr != target)
            {
                tmp1.insert(*iter);
            }
        }

        expr_gen = tmp1;

        for (auto iter = U_expr_set.begin(); iter != U_expr_set.end(); ++iter)
        {
            if (iter->type == IR_EXPR_ARRAY)
            {
                if (target.find('[') != target.npos)
                {
                    std::string name = std::string(target.begin(), target.begin() + target.find("["));
                    if (name == iter->l_addr)
                    {
                        expr_kill.insert(*iter);
                    }
                }
                else if (target == iter->r_addr || target == iter->target)
                {
                    expr_kill.insert(*iter);
                }
            }
            else if (iter->type == IR_EXPR_COPY)
            {
                if (iter->target == target || iter->r_addr == target)
                    expr_kill.insert(*iter);
            }
            else if (iter->l_addr == target || iter->r_addr == target || target == iter->target)
            {
                expr_kill.insert(*iter);
            }
        }
    }

    bool exist_expr(std::set<IrExpr> &set, const IrExpr &expr)
    {
        for (auto &e : set)
            if (e.l_addr == expr.l_addr && e.r_addr == expr.r_addr && e.op == expr.op)
                return true;
        return false;
    }

    void find_expr_gen_kill(std::set<IrExpr> &U_expr_set)
    {
        for (IrStmt *stmt : stmts)
        {
            if (stmt == NULL || stmt->type == IR_TYPE_RETURN ||
                stmt->type == IR_TYPE_IF || stmt->type == IR_TYPE_GOTO || stmt->type == IR_TYPE_OTHER)
            {
                continue;
            }

            IrExpr ir_expr;
            stmt->get_expr(&ir_expr);
            switch (stmt->type)
            {
            case IR_TYPE_BINARY:
            case IR_TYPE_UNARY:
            {
                expr_gen.insert(ir_expr);
                for (auto e : expr_kill)
                {
                    if (e == ir_expr)
                    {
                        expr_kill.erase(e);
                        break;
                    }
                }
                kill_expr(stmt->target, U_expr_set);
                break;
            }

            case IR_TYPE_ASSIGN:
            {
                if (stmt->target.find('[') != stmt->target.npos)
                {
                    std::string target = std::string(stmt->target.begin(),
                                                     stmt->target.begin() + stmt->target.find("["));
                    kill_expr(stmt->target, U_expr_set);
                }
                else
                {
                    expr_gen.insert(ir_expr);
                    //expr_kill.erase(ir_expr);
                    for (auto e : expr_kill)
                    {
                        if (e == ir_expr)
                        {
                            expr_kill.erase(e);
                            break;
                        }
                    }
                    kill_expr(stmt->target, U_expr_set);
                    expr_gen.insert(ir_expr);
                }
                break;
            }

            case IR_TYPE_FUNC_CALL:
            {
                if (stmt->target != "")
                    kill_expr(stmt->target, U_expr_set);
                break;
            }
            default:
                break;
            }
        }
    }

    bool exist_def(const std::set<Definition> &set, const std::string &var)
    {
        for (auto def : set)
        {
            if (var == def.def_var)
            {
                return true;
            }
        }
        return false;
    }

    void find_def_gen_kill(std::set<Definition> &U_def)
    {
        for (int i = stmts.size() - 1; i >= 0; --i)
        {
            IrStmt *stmt = stmts[i];
            if (stmt == NULL || !stmt->has_definition())
            {
                continue;
            }

            Definition def;
            def.stmt = stmt;
            def.block = this;
            def.def_var = stmt->get_def_var();
            if (!exist_def(def_gen, stmt->get_def_var()))
                def_gen.insert(def);

            for (Definition kill : U_def)
            {
                if (kill != def && kill.def_var == def.def_var)
                {
                    def_kill.insert(kill);
                }
            }
        }
    }

    template <class T>
    class MyVector : public std::vector<T>
    {
    public:
        void insert(const T &e)
        {
            if (find(*this, e) == false)
                this->push_back(e);
        }
    };

    struct DAGNode
    {
        MyVector<DAGNode *> child;
        MyVector<DAGNode *> parent;
        MyVector<std::string> def_list;
        bool is_killed = false;
        bool is_useful = true;
        int type;
        //int intVal;
        std::string op;
        std::string var;
    };

    enum
    {
        //NODE_TYPE_COPY,
        NODE_TYPE_UNARY_OP,
        NODE_TYPE_BINARY_OP,
        NODE_TYPE_ARRAY_VALUE,  //=[]
        NODE_TYPE_ARRAY_ASSIGN, //[]=
        NODE_TYPE_IF,
        NODE_TYPE_GOTO,
        NODE_TYPE_FUNC,
        NODE_TYPE_FUNC_NONE_RETURN,
        NODE_TYPE_RETURN,
        NODE_TYPE_LEAVE
    };

    //std::set<DAGNode *> graph;
    std::vector<DAGNode *> graph;
    std::vector<DAGNode *> internal_nodes;
    std::set<DAGNode *> leave_nodes;
    void optimize()
    {
        if (stmts.size() == 0)
            return;
        for (IrStmt *stmt : stmts)
        {
            if (stmt->type == IR_TYPE_UNARY)
            {
                if (stmt->op == "+") //%a = +%b
                    new_node_copy(stmt);

                else //%a = -%b      %a = &%b
                    new_node_unary(stmt);
            }
            else if (stmt->type == IR_TYPE_ASSIGN)
            {
                if (stmt->target.find('[') != stmt->target.npos) //%a[%b] = %c
                {
                    new_node_array_assign(stmt);
                }
                else if (stmt->r_addr.find('[') != stmt->r_addr.npos) //%a = %b[%c]
                {
                    new_node_array_value(stmt);
                }
                else //%a = %b
                {
                    new_node_copy(stmt);
                }
            }
            else if (stmt->type == IR_TYPE_BINARY)
            {
                new_node_binary(stmt);
            }
            else if (stmt->type == IR_TYPE_IF)
            {
                new_node_if(stmt);
            }
            else if (stmt->type == IR_TYPE_RETURN)
            {
                new_node_return(stmt);
            }
            else if (stmt->type == IR_TYPE_GOTO)
            {
                new_node_goto(stmt);
            }
            else if (stmt->type == IR_TYPE_FUNC_CALL)
            {
                new_node_func(stmt);
            }
            else
            {
                tmp_stmts.push_back(stmt);
                tmp_stmts_used.push_back(true);
            }
        }
        /*
        for (IrStmt *stmt : stmts)
            std::cout<<stmt->to_string()<<'\n';*/
        /*
            for (int i = 0; i < tmp_stmts.size(); ++i)
            if (tmp_stmts_used[i])
                std::cout<<tmp_stmts[i]->to_string()<<'\n';*/

        delete_dead_stmt();
        /*
for (int i = 0; i < tmp_stmts.size(); ++i)
            if (tmp_stmts_used[i])
                std::cout<<tmp_stmts[i]->to_string()<<'\n';*/

        delete_useless_copy();
        /*
for (int i = 0; i < tmp_stmts.size(); ++i)
            if (tmp_stmts_used[i])
                std::cout<<tmp_stmts[i]->to_string()<<'\n';*/

        stmts.clear();
        for (int i = 0; i < tmp_stmts.size(); ++i)
            if (tmp_stmts_used[i])
                stmts.push_back(tmp_stmts[i]);

        //delete_common_subexpr();
    }

    bool is_expr_usable(const IrExpr &expr)
    {
        for (auto &in : expr_in)
        {
            if (expr.type == IR_EXPR_COPY && in.type == IR_EXPR_COPY)
            {
                if (expr.r_addr == in.r_addr && expr.target == in.target)
                    return true;
            }
            else if (expr.op == in.op && expr.l_addr == in.l_addr && expr.r_addr == in.r_addr)
            {
                return true;
            }
        }
        return false;
    }

    std::string get_expr_target(const std::set<IrExpr> &set, const IrExpr &expr)
    {
        for (auto &tmp : set)
        {
            if (tmp.r_addr == expr.r_addr && tmp.l_addr == expr.l_addr && tmp.op == expr.op)
            {
                return tmp.target;
            }
        }
        return "";
    }

    void delete_common_subexpr()
    {
        for (int i = 0; i < stmts.size(); ++i)
        {
            IrExpr expr;
            if (stmts[i]->get_expr(&expr) && is_expr_usable(expr))
            {
                bool is_redefined = false;
                for (int j = 0; j < i; j++)
                {
                    std::string use;
                    IrStmt *stmt = stmts[j];
                    switch (stmt->type)
                    {
                    case IR_TYPE_ASSIGN:
                    {
                        if (stmt->target.find('[') != stmt->target.npos)
                        {
                            use = get_array_name(stmt->target);
                        }
                        else
                        {
                            use = stmt->target;
                        }
                        break;
                    }
                    case IR_TYPE_BINARY:
                    case IR_TYPE_UNARY:
                    case IR_TYPE_FUNC_CALL:
                    {
                        if (stmt->target != "")
                            use = stmt->target;
                        break;
                    }
                    default:
                        break;
                    }
                    if (use != "")
                        if (use == expr.r_addr || use == expr.l_addr)
                            is_redefined = true;
                }
                if (!is_redefined)
                    delete_common_subexpr(i, expr);
            }
        }
    }

    void delete_common_subexpr(int cur_stmt_index, IrExpr &expr)
    {
        static int subexpr_tmp = 0;
        //int i = cur_index;
        std::set<BasicBlock *> expr_gen_node;
        std::set<BasicBlock *> used_node;
        std::vector<BasicBlock *> stack;
        stack.resize(100);
        int top = 0;
        stack[top++] = this;
        used_node.insert(this);
        while (top)
        {
            BasicBlock *cur = stack[--top];
            if (cur != this && get_expr_target(cur->expr_gen, expr) != "")
            {
                expr_gen_node.insert(cur);
                continue;
            }
            else
            {
                for (BasicBlock *pre : cur->predecessor)
                {
                    if (used_node.find(pre) == used_node.end() &&
                        expr_gen_node.find(pre) == expr_gen_node.end())
                    {
                        stack[top++] = pre;
                        used_node.insert(pre);
                    }
                }
            }
        }

        std::string target;
        if (expr_gen_node.size() == 1)
        {
            BasicBlock *pre = *(expr_gen_node.begin());
            target = get_expr_target(pre->expr_gen, expr);
        }
        else if (expr_gen_node.size() > 1)
        {
            target = std::string("%0") + std::to_string(subexpr_tmp++);
            for (BasicBlock *pre : expr_gen_node)
            {
                for (IrExpr tmp_expr : pre->expr_gen)
                {
                    if (tmp_expr == expr)
                    {
                        for (int i = 0; i < pre->stmts.size(); ++i)
                        {
                            if (tmp_expr.stmt == pre->stmts[i])
                            {
                                IrStmt *new_stmt = new IrStmt();
                                new_stmt->type = IR_TYPE_ASSIGN;
                                new_stmt->r_addr = target;
                                new_stmt->target = pre->stmts[i]->target;
                                pre->stmts[i]->target = target;
                                pre->active_var_out.insert(target);
                                insert(pre->stmts, i + 1, new_stmt);
                                break;
                            }
                        }
                    }
                }
            }
        }
        if (expr_gen_node.size() > 0)
        {
            stmts[cur_stmt_index]->type = IR_TYPE_ASSIGN;
            stmts[cur_stmt_index]->r_addr = target;
        }
    }

    void delete_copy_spread()
    {
        for (int i = 0; i < stmts.size(); ++i)
        {
            IrStmt *stmt = stmts[i];
            if (stmt == NULL)
                continue;

            std::vector<std::string> use_list;
            switch (stmt->type)
            {
            case IR_TYPE_ASSIGN:
            {
                if (stmt->target.find('[') != stmt->target.npos)
                {
                    use_list.push_back(get_array_index(stmt->target));
                }
                else if (stmt->r_addr.find('[') != stmt->r_addr.npos)
                {
                    use_list.push_back(get_array_index(stmt->r_addr));
                }
                else
                {
                    use_list.push_back(stmt->r_addr);
                }
                break;
            }
            case IR_TYPE_BINARY:
            case IR_TYPE_IF:
            {
                use_list.push_back(stmt->l_addr);
                use_list.push_back(stmt->r_addr);
                break;
            }
            case IR_TYPE_UNARY:
            {
                use_list.push_back(stmt->r_addr);
                break;
            }
            case IR_TYPE_RETURN:
            {
                use_list.push_back(stmt->target);
                break;
            }
            case IR_TYPE_FUNC_CALL:
            {
                for (auto arg : stmt->func_args)
                    use_list.push_back(arg);
                break;
            }
            default:
                break;
            }

            for (std::string use : use_list)
            {
                if (type(use) == VAR_TYPE_GLOBAL)
                    continue;
                bool is_redefine = false;
                for (int j = 0; j < i; j++)
                    if (stmts[j]->target == use)
                        is_redefine = true;
                if (is_redefine)
                    continue;

                for (IrExpr expr : expr_in)
                {
                    if (expr.type == IR_EXPR_COPY && expr.target == use)
                    {
                        try_var_replace(stmt, use, expr.r_addr);
                        break;
                    }
                }
            }
        }
    }

    void delete_useless_copy()
    {
        std::vector<int> used_index;
        for (int i = 0; i < tmp_stmts_used.size(); ++i)
            if (tmp_stmts_used[i])
                used_index.push_back(i);
        if (used_index.size() == 0)
            return;
        for (int i = 0; i < used_index.size() - 1; ++i)
        {
            IrStmt *first = tmp_stmts[used_index[i]];
            IrStmt *second = tmp_stmts[used_index[i + 1]];

            if (first->type == IR_TYPE_ASSIGN && first->target == first->r_addr)
            {
                tmp_stmts_used[used_index[i]] = false;
                continue;
            }

            if (first->target != "" && first->target == second->r_addr && second->op == "" &&
                first->target.find('[') == first->target.npos && second->type == IR_TYPE_ASSIGN &&
                second->target.find('[') == second->target.npos)
            {
                if (active_var_out.find(first->target) != active_var_out.end())
                {
                    continue;
                }

                bool is_used = false;
                for (int j = i + 2; j < used_index.size(); ++j)
                {
                    try_var_replace(tmp_stmts[used_index[j]], first->target, second->target);
                    if (tmp_stmts[used_index[j]]->target == second->target)
                    {
                        break;
                    }
                    if (is_var_used(tmp_stmts[used_index[j]], first->target))
                    {
                        is_used = true;
                        break;
                    }
                }
                //is_used |= is_var_used(tmp_stmts[used_index[j]], first->target);
                if (is_used)
                {
                    continue;
                }
                first->target = second->target;
                tmp_stmts_used[used_index[i + 1]] = false;
            }
        }

        for (int i = 0; i < used_index.size() - 1; ++i)
        {
            IrStmt *first = tmp_stmts[used_index[i]];
            IrStmt *second = tmp_stmts[used_index[i + 1]];

            if (first->type == IR_TYPE_ASSIGN && second->type == IR_TYPE_ASSIGN &&
                first->target == second->r_addr && first->target.find('[') != first->target.npos)
            {
                bool is_used = false;
                for (int j = i + 2; j < used_index.size(); ++j)
                {
                    IrStmt *cur_stmt = tmp_stmts[used_index[j]];
                    try_var_replace(cur_stmt, second->target, first->r_addr);
                    if (first->r_addr == cur_stmt->target) //redefine
                    {
                        break;
                    }
                    if (cur_stmt->target == second->target)
                    {
                        is_used = true;
                        break;
                    }
                }
                tmp_stmts_used[used_index[i + 1]] = is_used | is_active(second->target);
            }
        }
    }

    bool is_active(const std::string &var)
    {
        if (active_var_out.find(var) != active_var_out.end())
            return true;
        return false;
    }

    void delete_dead_var(DAGNode *node)
    {
        //std::vector<std::string> vec = to_array(node->def_list);
        //for (std::string &var : vec)
        for (std::vector<std::string>::iterator iter = node->def_list.begin(); iter != node->def_list.end(); ++iter)
        {
            if (active_var_out.find(*iter) == active_var_out.end())
            {
                if (node->def_list.size() == 1 && node->type != NODE_TYPE_LEAVE &&
                    node->parent.size() != 0)
                    continue;
                if (node->type == NODE_TYPE_IF || node->type == NODE_TYPE_GOTO)
                    continue;
                if (node->type == NODE_TYPE_LEAVE && node->def_list.size() == 1 && node->var == "")
                    continue;

                node->def_list.erase(iter);
                --iter;
            }
        }
    }

    bool delete_dead_internal_node(DAGNode *node)
    {
        if (node->def_list.size() == 0)
        {
            int type = node->type;
            if (type == NODE_TYPE_RETURN || type == NODE_TYPE_FUNC ||
                type == NODE_TYPE_ARRAY_ASSIGN || type == NODE_TYPE_IF ||
                type == NODE_TYPE_FUNC_NONE_RETURN || type == NODE_TYPE_GOTO)
            {
                return false;
            }
            if (node->def_list.size() == 0)
            {
                if (type == NODE_TYPE_UNARY_OP)
                {
                    erase(node->child[0]->parent, node);
                    node->child.clear();
                }
                else
                {
                    erase(node->child[0]->parent, node);
                    erase(node->child[1]->parent, node);
                    node->child.clear();
                }
                erase(internal_nodes, node);
                return true;
            }
        }
        return false;
    }

    void delete_dead_leave_node(DAGNode *node)
    {
        if (node->type != NODE_TYPE_GOTO &&
            node->def_list.size() == 0 && node->parent.size() == 0)
        {
            leave_erase(node);
            //leave_nodes.erase(node);
        }
    }

    bool is_var_used(IrStmt *stmt, const std::string &var)
    {
        if (stmt->target.find('[') != stmt->target.npos)
            if (var == get_array_index(stmt->target))
                return true;

        if (stmt->r_addr.find('[') != stmt->r_addr.npos)
            if (var == get_array_index(stmt->r_addr))
                return true;

        if (stmt->l_addr == var || stmt->r_addr == var)
            return true;

        if (stmt->type == IR_TYPE_RETURN && stmt->target == var)
            return true;

        for (auto arg : stmt->func_args)
            if (arg == var)
                return true;

        return false;
    }

    void try_var_replace(IrStmt *stmt, const std::string &src, const std::string &dest)
    {
        if (stmt->target.find('[') != stmt->target.npos)
            if (src == get_array_index(stmt->target))
                stmt->target = get_array_name(stmt->target) + '[' + dest + ']';

        if (stmt->r_addr.find('[') != stmt->r_addr.npos)
            if (src == get_array_index(stmt->r_addr))
                stmt->r_addr = get_array_name(stmt->r_addr) + '[' + dest + ']';

        if (stmt->l_addr == src)
            stmt->l_addr = dest;

        if (stmt->r_addr == src)
            stmt->r_addr = dest;

        if (stmt->type == IR_TYPE_RETURN && stmt->target == src)
            stmt->target = dest;

        for (auto &arg : stmt->func_args)
            if (arg == src)
                arg = dest;
    }

    void delete_dead_stmt()
    {
        bool is_changed = true;
        while (is_changed)
        {
            is_changed = false;
            int index = 0;
            for (std::vector<IrStmt *>::iterator iter = tmp_stmts.begin();
                 iter != tmp_stmts.end(); ++iter, ++index)
            {
                if (tmp_stmts_used[index] == false)
                    continue;
                if ((*iter)->type == IR_TYPE_FUNC_CALL || (*iter)->type == IR_TYPE_GOTO ||
                    (*iter)->type == IR_TYPE_IF || (*iter)->type == IR_TYPE_RETURN ||
                    (*iter)->type == IR_TYPE_OTHER)
                {
                    continue;
                }

                std::string target = (*iter)->target;
                if (active_var_out.find(target) != active_var_out.end() ||
                    target.find('[') != target.npos)
                {
                    continue;
                }

                bool is_used = false;
                int n = index;
                for (auto next = iter; next != tmp_stmts.end(); ++next, ++n)
                    if (tmp_stmts_used[n])
                        is_used |= is_var_used(*next, target);

                tmp_stmts_used[index] = is_used;
                if (is_used == false)
                    is_changed = true;
            }
        }
    }

    void leave_erase(DAGNode *node)
    {
        std::set<DAGNode *>::iterator iter = leave_nodes.find(node);
        if (iter != std::end(leave_nodes))
        {
            //(*iter) = NULL;
            //leave_nodes.erase(iter);
            node->is_useful = false;
        }
    }

    void graph_to_stmt()
    {
        for (IrStmt *p : stmts)
            delete p;
        stmts.clear();

        IrStmt *jump;

        for (DAGNode *node : graph)
        {
            if (node->type == NODE_TYPE_LEAVE)
            {
                if (node->is_useful && node->def_list.size() != 0 && node->parent.size() == 0)
                {
                    copy_node(node, "var");
                    if (node->def_list.size() == 0)
                        leave_erase(node);
                }
            }
            else
            {
                IrStmt *stmt = new IrStmt();
                std::string target = "";
                int child_num = 0;
                if (node->type == NODE_TYPE_UNARY_OP)
                {
                    stmt->type = IR_TYPE_UNARY;
                    stmt->target = *(node->def_list.begin());
                    stmt->op = node->op;
                    stmt->r_addr = get_var(node->child[0]);
                    child_num = 1;
                    target = stmt->target;
                }
                else if (node->type == NODE_TYPE_BINARY_OP)
                {
                    stmt->type = IR_TYPE_BINARY;
                    stmt->target = *(node->def_list.begin());
                    stmt->op = node->op;
                    stmt->l_addr = get_var(node->child[0]);
                    stmt->r_addr = get_var(node->child[1]);
                    child_num = 2;
                    target = stmt->target;
                }
                else if (node->type == NODE_TYPE_ARRAY_VALUE)
                {
                    stmt->type = IR_TYPE_ASSIGN;
                    stmt->target = *(node->def_list.begin());
                    stmt->r_addr = get_var(node->child[0]) + "[" + get_var(node->child[1]) + "]";
                    child_num = 2;
                    target = stmt->target;
                }
                else if (node->type == NODE_TYPE_ARRAY_ASSIGN)
                {
                    stmt->type = IR_TYPE_ASSIGN;
                    stmt->r_addr = get_var(node->child[2]);
                    stmt->target = get_var(node->child[0]) + "[" + get_var(node->child[1]) + "]";
                    child_num = 3;
                }
                else if (node->type == NODE_TYPE_RETURN)
                {
                    stmt->type = IR_TYPE_RETURN;
                    stmt->target = get_var(node->child[0]);
                    child_num = 1;
                }
                else if (node->type == NODE_TYPE_GOTO)
                {
                    stmt->type = IR_TYPE_GOTO;
                    stmt->target = *(node->def_list.begin());
                    child_num = 0;
                }
                else if (node->type == NODE_TYPE_IF)
                {
                    stmt->type = IR_TYPE_IF;
                    stmt->target = *(node->def_list.begin());
                    stmt->op = node->op;
                    stmt->l_addr = get_var(node->child[0]);
                    stmt->r_addr = get_var(node->child[1]);
                    child_num = 2;
                }
                else if (node->type == NODE_TYPE_FUNC_NONE_RETURN || node->type == NODE_TYPE_FUNC)
                {
                    stmt->type = IR_TYPE_FUNC_CALL;
                    int start_index = 0;
                    if (node->type == NODE_TYPE_FUNC)
                    {
                        stmt->target = get_var(node->child[0]);
                        start_index = 1;
                        child_num++;
                        target = stmt->target;
                    }
                    for (int i = start_index; i < node->child.size(); ++i)
                    {
                        stmt->func_args.push_back(get_var(node->child[i]));
                        child_num++;
                    }
                    stmt->func_name = node->op;
                }

                if (target != "")
                {
                    for (DAGNode *n : leave_nodes)
                    {
                        if (n->is_useful && n->var == target)
                        {
                            copy_node(n, "var");
                            n->def_list.clear();
                        }
                    }
                }

                if (node->type == NODE_TYPE_UNARY_OP || node->type == NODE_TYPE_BINARY_OP || node->type == NODE_TYPE_ARRAY_VALUE || node->type == NODE_TYPE_ARRAY_ASSIGN)
                {
                    stmts.push_back(stmt);

                    copy_node(node, "def");
                    for (int i = 0; i < child_num; ++i)
                    {
                        DAGNode *tmp = *(node->child.begin());
                        erase(tmp->parent, node);
                        erase(node->child, tmp);
                        if (tmp->parent.size() == 0)
                        {
                            copy_node(tmp, "var");
                            leave_erase(tmp);
                        }
                    }
                }
                else
                {
                    for (int i = 0; i < child_num; ++i)
                    {
                        DAGNode *tmp = *(node->child.begin());
                        erase(tmp->parent, node);
                        //node->child.erase(node->child.begin());
                        erase(node->child, tmp);
                        if (tmp->parent.size() == 0)
                        {
                            copy_node(tmp, "var");
                            leave_erase(tmp);
                        }
                    }
                    stmts.push_back(stmt);
                    copy_node(node, "def");
                }

                for (DAGNode *node : leave_nodes)
                {
                    if (node->is_useful && node->def_list.size() != 0 && node->parent.size() == 0)
                    {
                        copy_node(node, "var");
                        leave_erase(node);
                    }
                }
            }
        }
    }

private:
    void graph_to_stmt_unary(DAGNode *node)
    {
        IrStmt *stmt = new IrStmt();
        stmt->type = IR_TYPE_UNARY;
        stmt->target = *(node->def_list.begin());
        stmt->op = node->op;
        stmt->r_addr = get_var(node->child[0]);
        stmts.push_back(stmt);
        if (node->child[0]->def_list.size() != 0)
        {
            copy_node(node->child[0], "var");
        }
    }

    void copy_node(DAGNode *node, const std::string &flag)
    {
        if (flag == "def")
        {
            if (node->def_list.size() <= 1)
                return;
            std::vector<std::string> &vec = node->def_list;
            for (int i = 1; i < vec.size(); i++)
            {
                IrStmt *stmt = new IrStmt();
                std::string target = vec[i];
                for (DAGNode *n : leave_nodes)
                {
                    if (n->is_useful && n->var == target)
                    {
                        copy_node(n, "var");
                        n->def_list.clear();
                    }
                }
                stmt->target = target;
                stmt->r_addr = vec[0];
                stmt->type = IR_TYPE_ASSIGN;
                stmts.push_back(stmt);
            }
        }
        else if (flag == "var" && node->var != "" && node->var != "=[]")
        {
            //for (auto var : node->def_list)
            for (std::vector<std::string>::iterator iter = node->def_list.begin(); iter != node->def_list.end(); ++iter)
            {
                bool flag = true;
                for (DAGNode *n : leave_nodes)
                    if (n->is_useful && n->var == *iter)
                        flag = false;

                if (flag == false)
                    continue;

                IrStmt *stmt = new IrStmt();
                stmt->type = IR_TYPE_ASSIGN;
                stmt->target = *iter;
                stmt->r_addr = node->var;
                stmts.push_back(stmt);

                //erase(node->def_list, var);
                node->def_list.erase(iter);
                --iter;
            }
        }
    }

    std::string get_var(DAGNode *node)
    {
        if (node->var == "" && node->def_list.size() == 0)
            return "";
        if (node->type == NODE_TYPE_LEAVE && node->var != "")
            return node->var;
        /*
        for (auto var : node->def_list)
            if (active_var_out.find(var) != active_var_out.end())
                return var;

        for (auto var : node->def_list)
            if (type(var) != VAR_TYPE_TEMP)
                return var;*/

        return *(node->def_list.begin());
    }

    bool is_digit(std::string &var)
    {
        if (var[0] == '#')
            return true;
        return false;
    }

    int to_digit(std::string &var)
    {
        return atoi(var.c_str() + 1);
    }

    bool is_digit(DAGNode *n)
    {
        return is_digit(n->var);
    }

    int to_digit(DAGNode *n)
    {
        return to_digit(n->var);
    }

    void add_to_graph(DAGNode *node)
    {
        //graph.insert(node);
        if (find(graph, node) == false)
            graph.push_back(node);
    }

    IrStmt *new_stmt(const std::string &target, const std::string &l_addr, const std::string &r_addr,
                     const std::string &op, int type)
    {
        IrStmt *stmt = new IrStmt();
        stmt->target = target;
        stmt->type = type;
        stmt->l_addr = l_addr;
        stmt->r_addr = r_addr;
        stmt->op = op;
        return stmt;
    }

    //std::vector<IrStmt*> copy_stmt;
    std::vector<bool> tmp_stmts_used;
    std::vector<IrStmt *> tmp_stmts;

    //包括了%a=%b  %a=+%b  %a=#1三种情况
    void new_node_copy(IrStmt *stmt)
    {
        DAGNode *n = node_var(stmt->r_addr);
        if (n == NULL)
        {
            n = new_node_leave(stmt->r_addr);
            //add_to_graph(n);
            add_to_graph(n);
        }

        n->def_list.insert(stmt->target);
        delete_useless_assign(stmt->target, n);

        tmp_stmts.push_back(new_stmt(stmt->target, "", get_var(n), "", IR_TYPE_ASSIGN));
        tmp_stmts_used.push_back(true);
    }

    //包括了%a=-%b  %a=&%b  %a=-#1三种情况
    void new_node_unary(IrStmt *stmt)
    {
        DAGNode *target;
        DAGNode *r_addr = node_var(stmt->r_addr);
        if (r_addr == NULL) //操作数不存在
        {
            //新建操作数节点，并进行标记
            r_addr = new_node_leave(stmt->r_addr);
            if (is_digit(r_addr)) //操作数是一个常数
            {
                int num = -to_digit(r_addr);
                target = new_node_leave(std::string("#") + std::to_string(num));
                //add_to_graph(target);
                add_to_graph(target);
                target->def_list.insert(stmt->target);

                tmp_stmts.push_back(new_stmt(stmt->target, "", get_var(target), "", IR_TYPE_ASSIGN));
                tmp_stmts_used.push_back(true);

                delete r_addr;
            }
            else
            {
                //add_to_graph(r_addr);
                add_to_graph(r_addr);

                //操作数节点不存在的情况下，其父节点也需要新建

                target = new_node_op(stmt->op, NODE_TYPE_UNARY_OP);
                //add_to_graph(target);
                add_to_graph(target);

                //将目的地址加入定值变量表
                target->def_list.insert(stmt->target);

                //确定两节点的父子关系
                target->child.push_back(r_addr);
                r_addr->parent.push_back(target);

                tmp_stmts.push_back(new_stmt(stmt->target, "", get_var(r_addr), stmt->op, IR_TYPE_UNARY));
                tmp_stmts_used.push_back(true);
            }
        }
        else //操作数存在
        {
            if (is_digit(r_addr->var)) //操作数是一个常数
            {
                //假定当右操作数为常数时，运算符只有“-”，“+”已提前处理
                int num = -to_digit(r_addr);
                target = new_node_leave(std::string("#") + std::to_string(num));
                //add_to_graph(target);
                add_to_graph(target);
                target->def_list.insert(stmt->target);

                tmp_stmts.push_back(new_stmt(stmt->target, "", get_var(target), "", IR_TYPE_ASSIGN));
                tmp_stmts_used.push_back(true);
            }
            else //操作数是一个变量
            {
                target = node_op(stmt->op, NULL, r_addr, 1);
                if (target == NULL || find(target->child, r_addr) == false)
                {
                    target = new_node_op(stmt->op, NODE_TYPE_UNARY_OP);
                    //add_to_graph(target);
                    add_to_graph(target);
                    target->child.push_back(r_addr);
                    r_addr->parent.push_back(target);
                    target->def_list.insert(stmt->target);
                    tmp_stmts.push_back(new_stmt(stmt->target, "", get_var(r_addr), stmt->op, IR_TYPE_UNARY));
                    tmp_stmts_used.push_back(true);
                }
                else
                {
                    target->def_list.insert(stmt->target);
                    tmp_stmts.push_back(new_stmt(stmt->target, "", get_var(target), "", IR_TYPE_ASSIGN));
                    tmp_stmts_used.push_back(true);
                }
            }
        }
        delete_useless_assign(stmt->target, target);
    }

    void new_node_binary(IrStmt *stmt)
    {
        DAGNode *l_addr = node_var(stmt->l_addr);
        if (l_addr == NULL)
            l_addr = new_node_leave(stmt->l_addr);

        DAGNode *r_addr = node_var(stmt->r_addr);
        if (r_addr == NULL)
            r_addr = new_node_leave(stmt->r_addr);

        DAGNode *target;
        if (is_digit(l_addr) && is_digit(r_addr))
        {
            int num;
            if (stmt->op == "+")
                num = to_digit(l_addr) + to_digit(r_addr);
            else if (stmt->op == "-")
                num = to_digit(l_addr) - to_digit(r_addr);
            else if (stmt->op == "*")
                num = to_digit(l_addr) * to_digit(r_addr);
            else if (stmt->op == "/")
                num = to_digit(l_addr) / to_digit(r_addr);
            else if (stmt->op == "mod")
                num = to_digit(l_addr) % to_digit(r_addr);

            std::string var = std::string("#") + std::to_string(num);
            target = node_var(var);
            if (target == NULL)
            {
                target = new_node_leave(var);
                add_to_graph(target);
            }
            target->def_list.insert(stmt->target);

            tmp_stmts.push_back(new_stmt(stmt->target, "", var, "", IR_TYPE_ASSIGN));
            tmp_stmts_used.push_back(true);

            delete_useless_assign(stmt->target, target);
        }
        else if (is_digit(l_addr))
        {
            int num = to_digit(l_addr);
            if (stmt->op == "*" && num == 0 || stmt->op == "/" && num == 0)
            {
                target = node_var("#0");
                if (target == NULL)
                {
                    target = new_node_leave("#0");
                    add_to_graph(target);
                }
                target->def_list.insert(stmt->target);

                tmp_stmts.push_back(new_stmt(stmt->target, "", "#0", "", IR_TYPE_ASSIGN));
                tmp_stmts_used.push_back(true);

                delete_useless_assign(stmt->target, target);
            }
            else if (stmt->op == "*" && num == 1 || stmt->op == "+" && num == 0)
            {
                r_addr->def_list.insert(stmt->target);
                add_to_graph(r_addr);

                tmp_stmts.push_back(new_stmt(stmt->target, "", get_var(r_addr), "", IR_TYPE_ASSIGN));
                tmp_stmts_used.push_back(true);

                delete_useless_assign(stmt->target, r_addr);
            }
            else
            {
                add_to_graph(l_addr);
                add_to_graph(r_addr);

                target = node_op(stmt->op, l_addr, r_addr, 2);
                if (target == NULL)
                {
                    target = new_node_op(stmt->op, NODE_TYPE_BINARY_OP);
                    add_to_graph(target);

                    target->child.push_back(l_addr);
                    target->child.push_back(r_addr);
                    l_addr->parent.push_back(target);
                    r_addr->parent.push_back(target);
                    target->def_list.insert(stmt->target);

                    tmp_stmts.push_back(new_stmt(stmt->target, get_var(l_addr), get_var(r_addr), stmt->op, IR_TYPE_BINARY));
                    tmp_stmts_used.push_back(true);
                }
                else
                {
                    target->def_list.insert(stmt->target);

                    tmp_stmts.push_back(new_stmt(stmt->target, "", get_var(target), "", IR_TYPE_ASSIGN));
                    tmp_stmts_used.push_back(true);
                }
                delete_useless_assign(stmt->target, target);
            }
        }
        else if (is_digit(r_addr))
        {
            int num = to_digit(r_addr);
            if (stmt->op == "*" && num == 0)
            {
                target = node_var(r_addr->var);
                if (target == NULL)
                {
                    target = new_node_leave("#0");
                    add_to_graph(target);
                }
                target->def_list.insert(stmt->target);

                tmp_stmts.push_back(new_stmt(stmt->target, "", "#0", "", IR_TYPE_ASSIGN));
                tmp_stmts_used.push_back(true);

                delete_useless_assign(stmt->target, target);
            }
            else if (stmt->op == "*" && num == 1 || stmt->op == "+" && num == 0 || stmt->op == "-" && num == 0)
            {
                l_addr->def_list.insert(stmt->target);
                add_to_graph(l_addr);

                tmp_stmts.push_back(new_stmt(stmt->target, "", get_var(l_addr), "", IR_TYPE_ASSIGN));
                tmp_stmts_used.push_back(true);

                delete_useless_assign(stmt->target, l_addr);
            }
            else
            {
                add_to_graph(l_addr);
                add_to_graph(r_addr);

                target = node_op(stmt->op, l_addr, r_addr, 2);
                if (target == NULL || target->child[0] != l_addr || target->child[1] != r_addr)
                {
                    target = new_node_op(stmt->op, NODE_TYPE_BINARY_OP);
                    add_to_graph(target);
                    target->child.push_back(l_addr);
                    target->child.push_back(r_addr);
                    l_addr->parent.push_back(target);
                    r_addr->parent.push_back(target);
                    target->def_list.insert(stmt->target);

                    tmp_stmts.push_back(new_stmt(stmt->target, get_var(l_addr), get_var(r_addr), stmt->op, IR_TYPE_BINARY));
                    tmp_stmts_used.push_back(true);
                }
                else
                {
                    target->def_list.insert(stmt->target);

                    tmp_stmts.push_back(new_stmt(stmt->target, "", get_var(target), "", IR_TYPE_ASSIGN));
                    tmp_stmts_used.push_back(true);
                }

                delete_useless_assign(stmt->target, target);
            }
        }
        else
        {
            //if (l_addr->var == r_addr->var && stmt->op == "-") //%a=%b-%b -> %a=#0
            if (get_var(l_addr) == get_var(r_addr) && stmt->op == "-") //%a=%b-%b -> %a=#0
            {
                target = node_var("#0");
                if (target == NULL)
                {
                    target = new_node_leave("#0");
                    add_to_graph(target);
                }
                target->def_list.insert(stmt->target);

                tmp_stmts.push_back(new_stmt(stmt->target, "", "#0", "", IR_TYPE_ASSIGN));
                tmp_stmts_used.push_back(true);
            }
            else
            {
                target = node_op(stmt->op, l_addr, r_addr, 2);
                if (target == NULL)
                {
                    target = new_node_op(stmt->op, NODE_TYPE_BINARY_OP);
                    add_to_graph(target);
                    add_to_graph(l_addr);
                    add_to_graph(r_addr);
                    target->child.push_back(l_addr);
                    target->child.push_back(r_addr);
                    l_addr->parent.push_back(target);
                    r_addr->parent.push_back(target);
                    target->def_list.insert(stmt->target);

                    tmp_stmts.push_back(new_stmt(stmt->target, get_var(l_addr), get_var(r_addr), stmt->op, IR_TYPE_BINARY));
                    tmp_stmts_used.push_back(true);
                }
                else
                {
                    target->def_list.insert(stmt->target);

                    tmp_stmts.push_back(new_stmt(stmt->target, "", get_var(target), "", IR_TYPE_ASSIGN));
                    tmp_stmts_used.push_back(true);
                }
            }
            delete_useless_assign(stmt->target, target);
        }
    }

    //%a[%b] = %c
    void new_node_array_assign(IrStmt *stmt)
    {
        std::string var1 = std::string(stmt->target.begin(),
                                       stmt->target.begin() + stmt->target.find("["));
        std::string var2 = std::string(stmt->target.begin() + stmt->target.find("[") + 1,
                                       stmt->target.end() - 1);
        std::string var3 = stmt->r_addr;

        DAGNode *addr1 = node_var(var1);
        if (addr1 != NULL)
            for (DAGNode *p : addr1->parent)
                p->is_killed = true;
        else
        {
            addr1 = new_node_leave(var1);
            add_to_graph(addr1);
        }

        DAGNode *addr2 = node_var(var2);
        if (addr2 == NULL)
        {
            addr2 = new_node_leave(var2);
            add_to_graph(addr2);
        }

        DAGNode *addr3 = node_var(var3);
        if (addr3 == NULL)
        {
            addr3 = new_node_leave(var3);
            add_to_graph(addr3);
        }

        DAGNode *target = node_op("[]=", addr1, addr2, 2);
        if (target == NULL || target->child[2] != addr3)
        {
            target = new_node_op("[]=", NODE_TYPE_ARRAY_ASSIGN);
            add_to_graph(target);
            target->child.push_back(addr1);
            target->child.push_back(addr2);
            target->child.push_back(addr3);
            addr1->parent.push_back(target);
            addr2->parent.push_back(target);
            addr3->parent.push_back(target);

            tmp_stmts.push_back(new_stmt(get_var(addr1) + "[" + get_var(addr2) + "]", "", get_var(addr3), "", IR_TYPE_ASSIGN));
            tmp_stmts_used.push_back(true);
        }
    }

    //%a = %b[%c]
    void new_node_array_value(IrStmt *stmt)
    {
        std::string l_var = std::string(stmt->r_addr.begin(),
                                        stmt->r_addr.begin() + stmt->r_addr.find("["));
        std::string r_var = std::string(stmt->r_addr.begin() + stmt->r_addr.find("[") + 1,
                                        stmt->r_addr.end() - 1);

        bool is_exist = true;
        DAGNode *l_addr = node_var(l_var);
        if (l_addr == NULL)
        {
            l_addr = new_node_leave(l_var);
            add_to_graph(l_addr);
            is_exist = false;
        }

        DAGNode *r_addr = node_var(r_var);
        if (r_addr == NULL)
        {
            r_addr = new_node_leave(r_var);
            add_to_graph(r_addr);
            is_exist = false;
        }

        if (is_exist)
        {
            DAGNode *parent = node_op("[]=", l_addr, r_addr, 2);
            if (parent != NULL)
            {
                parent->child[2]->def_list.insert(stmt->target);
                delete_useless_assign(stmt->target, parent->child[2]);

                tmp_stmts.push_back(new_stmt(stmt->target, "", get_var(parent->child[2]), "", IR_TYPE_ASSIGN));
                tmp_stmts_used.push_back(true);

                return;
            }
        }

        DAGNode *target = node_op("=[]", l_addr, r_addr, 2);
        //if (target == NULL || target->is_killed || target->child[0] != l_addr || target->child[1] != r_addr)
        if (target == NULL || target->is_killed)
        {
            target = new_node_op("=[]", NODE_TYPE_ARRAY_VALUE);
            add_to_graph(target);
            target->child.push_back(l_addr);
            target->child.push_back(r_addr);
            l_addr->parent.push_back(target);
            r_addr->parent.push_back(target);
            target->def_list.insert(stmt->target);
            delete_useless_assign(stmt->target, target);

            tmp_stmts.push_back(new_stmt(stmt->target, "", get_var(l_addr) + "[" + get_var(r_addr) + "]", "", IR_TYPE_ASSIGN));
            tmp_stmts_used.push_back(true);
        }
        else
        {
            target->def_list.insert(stmt->target);
            delete_useless_assign(stmt->target, target);

            tmp_stmts.push_back(new_stmt(stmt->target, "", get_var(target), "", IR_TYPE_ASSIGN));
            tmp_stmts_used.push_back(true);
        }
    }

    void new_node_if(IrStmt *stmt)
    {
        DAGNode *l_addr = node_var(stmt->l_addr);
        if (l_addr == NULL)
            l_addr = new_node_leave(stmt->l_addr);

        DAGNode *r_addr = node_var(stmt->r_addr);
        if (r_addr == NULL)
            r_addr = new_node_leave(stmt->r_addr);

        DAGNode *target = new_node_op(stmt->op, NODE_TYPE_IF);
        add_to_graph(target);
        add_to_graph(l_addr);
        add_to_graph(r_addr);
        target->child.push_back(l_addr);
        target->child.push_back(r_addr);
        l_addr->parent.push_back(target);
        r_addr->parent.push_back(target);
        target->def_list.insert(stmt->target);

        tmp_stmts.push_back(new_stmt(stmt->target, get_var(l_addr), get_var(r_addr), stmt->op, IR_TYPE_IF));
        tmp_stmts_used.push_back(true);
    }

    void new_node_goto(IrStmt *stmt)
    {
        DAGNode *target = new_node_op(stmt->op, NODE_TYPE_GOTO);
        add_to_graph(target);
        target->def_list.insert(stmt->target);

        tmp_stmts.push_back(new_stmt(stmt->target, "", "", "goto", IR_TYPE_GOTO));
        tmp_stmts_used.push_back(true);
    }

    DAGNode *connect(DAGNode *parent, const std::string &var)
    {
        DAGNode *target = node_var(var);
        if (target == NULL)
        {
            target = new_node_leave(var);
            add_to_graph(target);
        }
        target->parent.push_back(parent);
        parent->child.push_back(target);
        return target;
    }

    void kill_global(DAGNode *node)
    {
        for (DAGNode *tmp : graph)
        {
            if (type(tmp->var) == VAR_TYPE_GLOBAL)
                tmp->var = "";
            for (std::string s : tmp->def_list)
            {
                if (tmp != node && type(s) == VAR_TYPE_GLOBAL)
                {
                    if (tmp != NULL && tmp != node)
                    {
                        if (tmp->type == NODE_TYPE_LEAVE && tmp->var != "" ||
                            tmp->def_list.size() > 1 || tmp->parent.size() == 0)
                            erase(tmp->def_list, s);
                    }
                }
            }
        }
    }

    void new_node_func(IrStmt *stmt)
    {
        bool arg_all_matched = true;
        for (std::string &arg : stmt->func_args)
        {
            if (node_var(arg) == NULL)
            {
                arg_all_matched = false;
                break;
            }
        }

        bool exist_and_reducible = false;
        for (auto node : graph)
            if (node->op != "memset" && node->op == stmt->func_name &&
                find(reducible_func_list, stmt->func_name))
                exist_and_reducible = true;

        if (arg_all_matched && exist_and_reducible)
        {
            DAGNode *func_node;
            for (auto node : graph)
            {
                if (node->op == stmt->func_name)
                {
                    func_node = node;
                    break;
                }
            }
            func_node->child[0]->def_list.insert(stmt->target);
            delete_useless_assign(stmt->target, func_node->child[0]);

            tmp_stmts.push_back(new_stmt(stmt->target, "", get_var(func_node->child[0]), "", IR_TYPE_ASSIGN));
            tmp_stmts_used.push_back(true);
        }
        else
        {
            DAGNode *func_node = new_node_op(stmt->func_name, NODE_TYPE_FUNC_NONE_RETURN);
            IrStmt *stmt_func = new IrStmt();
            DAGNode *target_node = NULL;
            if (stmt->target != "")
            {
                //DAGNode *target = connect(func_node, stmt->target);
                DAGNode *target = new_node_leave("");
                add_to_graph(target);
                target->def_list.insert(stmt->target);
                target->parent.push_back(func_node);
                func_node->child.push_back(target);

                delete_useless_assign(stmt->target, target);
                func_node->type = NODE_TYPE_FUNC;

                stmt_func->target = stmt->target;

                target_node = target;
            }

            stmt_func->type = IR_TYPE_FUNC_CALL;
            stmt_func->func_name = stmt->func_name;
            for (std::string &arg : stmt->func_args)
            {
                DAGNode *target = node_var(arg);
                if (target == NULL)
                {
                    target = new_node_leave(arg);
                    add_to_graph(target);
                }
                target->parent.push_back(func_node);
                func_node->child.push_back(target);

                stmt_func->func_args.push_back(get_var(target));
                //connect(func_node, arg);
            }
            tmp_stmts.push_back(stmt_func);
            tmp_stmts_used.push_back(true);

            kill_global(target_node);
        }
    }

    void new_node_return(IrStmt *stmt)
    {
        DAGNode *target = node_var(stmt->target);
        if (target == NULL)
        {
            target = new_node_leave(stmt->target);
            add_to_graph(target);
        }
        DAGNode *op = new_node_op("return", NODE_TYPE_RETURN);
        op->child.push_back(target);
        target->parent.push_back(op);

        tmp_stmts.push_back(new_stmt(get_var(target), "", "", "return", IR_TYPE_RETURN));
        tmp_stmts_used.push_back(true);
    }

    DAGNode *new_node_op(const std::string &op, int type)
    {
        DAGNode *addr = new DAGNode();
        addr->op = op;
        addr->type = type;
        internal_nodes.push_back(addr);
        add_to_graph(addr);
        return addr;
    }

    DAGNode *new_node_leave(const std::string &var)
    {
        DAGNode *addr = new DAGNode();
        addr->var = var;
        addr->type = NODE_TYPE_LEAVE;
        leave_nodes.insert(addr);
        add_to_graph(addr);
        return addr;
    }

    void delete_useless_assign(const std::string &var, DAGNode *node)
    {
        DAGNode *n = NULL;
        for (DAGNode *tmp : graph)
            for (std::string s : tmp->def_list)
                if (tmp != node && s == var)
                {
                    n = tmp;
                    break;
                }

        if (n != NULL && n != node)
        {
            if (n->type == NODE_TYPE_LEAVE && n->var != "" ||
                n->def_list.size() > 1 || n->parent.size() == 0)
                erase(n->def_list, var);
        }

        n = NULL;
        for (DAGNode *tmp : graph)
            if (tmp->var == var)
                n = tmp;
        if (n != NULL)
            n->var = "";
    }

    DAGNode *node_op(const std::string &op, DAGNode *l_addr, DAGNode *r_addr, int num)
    {
        if (op == "call")
            return NULL;
        if (num == 2)
        {
            for (DAGNode *n : graph)
                if (n->op == op && n->child[0] == l_addr && n->child[1] == r_addr)
                    return n;
        }
        else if (num == 1)
        {
            for (DAGNode *n : graph)
                if (n->op == op && n->child[0] == r_addr)
                    return n;
        }
        return NULL;
    }

    DAGNode *node_var(const std::string &var)
    {
        for (DAGNode *n : graph)
            for (std::string s : n->def_list)
                if (s == var)
                    return n;

        for (DAGNode *n : graph)
            if (n->var == var)
                return n;

        return NULL;
    }
};

struct NaturalLoop
{
    std::vector<BasicBlock *> nodes;
    std::vector<std::pair<BasicBlock *, IrStmt *>> invariant;
};

class IrFunction
{
public:
    BasicBlock *entry = new BasicBlock();
    BasicBlock *exit = new BasicBlock();
    std::map<double, BasicBlock *> basic_blocks;
    std::vector<std::pair<BasicBlock *, BasicBlock *>> back_edge;
    std::vector<NaturalLoop> natural_loops;
    std::map<std::string, std::vector<Definition>> use_def_chain;
    static int label_num;

    std::vector<std::string> decls;
    std::vector<std::string> body;
    //std::vector<int> first_stmt;
    int first_stmt[1000];
    int stmt_top = 0;
    std::map<int, std::vector<std::string>> current_dest;
    std::map<int, std::string> unresolved_jump;

private:
    static std::string get_next_label()
    {
        return std::string(".L") + std::to_string(++label_num);
    }

    static std::string get_current_label()
    {
        return std::string(".L") + std::to_string(label_num);
    }

    bool is_decl = true;

public:
    IrFunction()
    {
        basic_blocks[-1] = entry;
        basic_blocks[-2] = exit;
    }

    void add(const std::string &ir)
    {
        if (is_decl && (decls.size() == 0 || (str_split(ir, " ").size() == 4 && str_split(ir, " ")[2] == "alloca")))
        {
            decls.push_back(ir);
        }
        else
        {
            is_decl = false;
            body.push_back(ir);
        }
    }

    void insert_block(BasicBlock *index_block, BasicBlock *src_block) //insert before index
    {
        auto keys = get_keys(basic_blocks);
        double new_index = -3;
        for (int i = 2; i < keys.size(); ++i)
        {
            if (basic_blocks[i] == index_block)
            {
                new_index = (keys[i] + keys[i - 1]) / 2;
                break;
            }
        }
        if (new_index != -3)
            basic_blocks.insert(std::make_pair(new_index, src_block));
    }

    void clear()
    {
        back_edge.clear();
        natural_loops.clear();
        use_def_chain.clear();
        for (auto pair : basic_blocks)
        {
            pair.second->clear();
        }
    }

    std::string to_string()
    {
        std::stringstream result;
        for (auto decl : decls)
            result << decl << "\n";
        result << "\n";
        for (auto pair : basic_blocks)
        {
            BasicBlock *block = pair.second;
            if (block->has_label)
                result << "label " << block->label << "\n";
            for (auto stmt : block->stmts)
                if (stmt != NULL)
                    result << stmt->to_string() << "\n";
            /*      
            IrStmt*last_stmt=block->get_last_stmt();
            if(last_stmt->type==IR_TYPE_GOTO||last_stmt->type==IR_TYPE_IF)
                printf("特殊标记");
            else 
                printf("正常");*/

            result << "\n";
        }
        return result.str();
    }

    void build_all_basic_block()
    {
        //找出所有跳转语句和目的地址
        for (int i = 0; i < body.size(); ++i)
        {
            std::vector<std::string> parts = str_split(body[i], " ");
            if (parts[0] == "label")
            {
                std::vector<std::string> dest;

                while ((parts = str_split(body[i], " "))[0] == "label")
                {
                    dest.push_back(parts[1]);
                    i++;
                }
                current_dest[i] = dest;
                //first_stmt.insert(i);
                //first_stmt.push_back(i);
                if (!find(first_stmt, i, stmt_top))
                    first_stmt[stmt_top++] = i;
                i--;
            }
            else if (parts[0] == "if")
            {
                unresolved_jump[i] = parts[5];
                if (str_split(body[i + 1], " ")[0] != "label")
                    //first_stmt.insert(i + 1);
                    //first_stmt.push_back(i + 1);
                    if (!find(first_stmt, i + 1, stmt_top))
                        first_stmt[stmt_top++] = i + 1;
            }
            else if (parts[0] == "goto")
            {
                std::vector<std::string> next = str_split(body[i + 1], " ");
                if (next[0] == "label" && next[1] == parts[1])
                {
                    body[i].clear();
                }
                else if (next[0] == "label" && next[1] != parts[1])
                {
                    unresolved_jump[i] = parts[1];
                }
                else if (next[0] != "label")
                {
                    unresolved_jump[i] = parts[1];
                    //first_stmt.insert(i + 1);
                    //first_stmt.push_back(i + 1);
                    if (!find(first_stmt, i + 1, stmt_top))
                        first_stmt[stmt_top++] = i + 1;
                }
            }
        }

        //将跳转语句和其目的地址关联起来
        for (auto dest_list : current_dest)
        {
            for (auto dest : dest_list.second)
            {
                //同一个目的地址可能对应多条跳转语句
                //keys代表这些跳转语句的下标
                std::vector<int> keys = get_keys_from_value(unresolved_jump, dest);
                for (int key : keys)
                {
                    unresolved_jump[key] = std::to_string(dest_list.first);
                }
            }
        }

        for (int i = 0; i < body.size(); ++i)
        {
            if (str_split(body[i], " ")[0] != "label")
            {
                //first_stmt.insert(i);
                //first_stmt.push_back(i);
                if (!find(first_stmt, i, stmt_top))
                    first_stmt[stmt_top++] = i;
                break;
            }
        }

        std::sort(first_stmt, first_stmt + stmt_top);
        /*
        for(int i=0;i<stmt_top;++i)
            printf("%d ",first_stmt[i]);
        printf("\n");*/

        BasicBlock *tmp_block = build_basic_block(0, false);
        entry->successor.push_back(tmp_block);
        tmp_block->predecessor.push_back(entry);
        //entry->successor.push_back(build_basic_block(first_stmt.begin(), false));
    }

    void active_var_analyse()
    {

        for (auto pair : basic_blocks)
        {
            pair.second->find_active_var_def_use();
        }

        bool change = true;
        do
        {
            change = false;
            for (auto pair : basic_blocks)
            {
                BasicBlock *block = pair.second;
                if (block == exit)
                    continue;
                for (auto sucessor : block->successor) //out集为后继节点的in集的并集
                {
                    std::set_union(block->active_var_out.begin(), block->active_var_out.end(),
                                   sucessor->active_var_in.begin(), sucessor->active_var_in.end(),
                                   std::inserter(block->active_var_out, block->active_var_out.begin()));
                }

                //in集为 use集并(out集与def集的差集)
                std::set<std::string> differ;
                std::set_difference(block->active_var_out.begin(), block->active_var_out.end(),
                                    block->active_var_def.begin(), block->active_var_def.end(),
                                    std::inserter(differ, differ.begin()));

                std::set<std::string> tmp_in;
                std::set_union(block->active_var_use.begin(), block->active_var_use.end(),
                               differ.begin(), differ.end(),
                               std::inserter(tmp_in, tmp_in.begin()));

                if (equal(tmp_in, block->active_var_in) == false)
                {
                    change = true;
                    block->active_var_in = tmp_in;
                }
                /*if (is_child(block->active_var_in, differ) == false ||
                    is_child(block->active_var_in, block->active_var_use) == false)
                {
                    change = true;
                    std::set_union(block->active_var_use.begin(), block->active_var_use.end(),
                                   differ.begin(), differ.end(),
                                   std::inserter(block->active_var_in, block->active_var_in.begin()));
                }*/
            }
        } while (change);
    }

    void usable_expr_analyse()
    {
        std::set<IrExpr> U_expr_set;
        for (auto pair : basic_blocks)
        {
            for (IrStmt *stmt : pair.second->stmts)
            {
                IrExpr ir_expr;
                if (stmt->get_expr(&ir_expr))
                {
                    U_expr_set.insert(ir_expr);
                }
            }
        }

        for (auto pair : basic_blocks)
        {
            pair.second->find_expr_gen_kill(U_expr_set);
        }

        for (auto pair : basic_blocks)
        {
            BasicBlock *block = pair.second;
            if (block != entry)
            {
                block->expr_out = U_expr_set;
            }
        }

        bool is_changed = true;
        while (is_changed)
        {
            is_changed = false;
            for (auto pair : basic_blocks)
            {
                BasicBlock *block = pair.second;
                if (block == entry)
                    continue;

                BasicBlock *tmp_block = *block->predecessor.begin();
                std::set<IrExpr> tmp_in = tmp_block->expr_out;

                for (BasicBlock *predecessor : block->predecessor)
                {
                    std::set<IrExpr> tmp;
                    std::set_intersection(tmp_in.begin(), tmp_in.end(),
                                          predecessor->expr_out.begin(), predecessor->expr_out.end(),
                                          std::inserter(tmp, tmp.begin()));
                    tmp_in = tmp;
                }

                block->expr_in = tmp_in;
                std::set<IrExpr> differ;
                std::set_difference(tmp_in.begin(), tmp_in.end(),
                                    block->expr_kill.begin(), block->expr_kill.end(),
                                    std::inserter(differ, differ.begin()));

                std::set<IrExpr> tmp_out;
                std::set_union(block->expr_gen.begin(), block->expr_gen.end(),
                               differ.begin(), differ.end(),
                               std::inserter(tmp_out, tmp_out.begin()));
                if (equal(tmp_out, block->expr_out) == false)
                {
                    is_changed = true;
                    block->expr_out = tmp_out;
                }
            }
        }
    }

    void optimize_basic_blocks()
    {
        for (auto pair : basic_blocks)
        {
            pair.second->optimize();
        }
    }

    void delete_common_subexpr()
    {
        for (auto pair : basic_blocks)
        {
            pair.second->delete_common_subexpr();
        }
    }

    void delete_copy_spread()
    {
        for (auto pair : basic_blocks)
        {
            pair.second->delete_copy_spread();
        }
    }

    void arrival_definition_analyze()
    {
        for (auto pair : basic_blocks)
        {
            BasicBlock *block = pair.second;
            for (IrStmt *stmt : block->stmts)
            {
            }
        }
        std::set<Definition> U_def;
        for (auto pair : basic_blocks)
        {
            BasicBlock *block = pair.second;
            for (IrStmt *stmt : block->stmts)
            {
                if (stmt->has_definition())
                {
                    Definition definition;
                    definition.stmt = stmt;
                    definition.block = block;
                    definition.def_var = stmt->get_def_var();
                    U_def.insert(definition);
                }
            }
        }

        for (auto pair : basic_blocks)
        {
            pair.second->find_def_gen_kill(U_def);
        }

        /*for (auto pair : basic_blocks)
        {
            std::cout<<"--------------------------------\n";
            pair.second->print_def_gen();
            std::cout<<"--------------\n";
            pair.second->print_def_kill();
            std::cout<<"--------------------------------\n";
        }*/

        bool is_changed = true;
        while (is_changed)
        {
            is_changed = false;
            for (auto pair : basic_blocks)
            {
                BasicBlock *block = pair.second;
                if (block == entry)
                    continue;

                for (auto pre : block->predecessor)
                {
                    std::set_union(block->def_in.begin(), block->def_in.end(),
                                   pre->def_out.begin(), pre->def_out.end(),
                                   std::inserter(block->def_in, block->def_in.begin()));
                }

                std::set<Definition> differ;
                std::set_difference(block->def_in.begin(), block->def_in.end(),
                                    block->def_kill.begin(), block->def_kill.end(),
                                    std::inserter(differ, differ.begin()));

                std::set<Definition> tmp_out;
                std::set_union(block->def_gen.begin(), block->def_gen.end(),
                               differ.begin(), differ.end(),
                               std::inserter(tmp_out, tmp_out.begin()));

                if (!equal(tmp_out, block->def_out))
                {
                    is_changed = true;
                    block->def_out = tmp_out;
                }
            }
        }

        /* for (auto pair : basic_blocks)
        {
            //std::cout<<"--------------------------------\n";
            pair.second->print_def_in();
        }*/

        for (auto pair : basic_blocks)
        {
            BasicBlock *block = pair.second;
            for (IrStmt *stmt : block->stmts)
            {
                if (stmt == NULL)
                    continue;
                auto use_list = stmt->get_use_list();
                for (std::string use_var : use_list)
                {
                    IrStmt *pre = NULL;
                    for (int i = 0; i < block->stmts.size(); ++i)
                    {
                        if (block->stmts[i] == NULL)
                            continue;
                        if (block->stmts[i] == stmt)
                            break;
                        if (block->stmts[i]->has_definition() == false)
                            continue;
                        if (use_var == block->stmts[i]->get_def_var())
                            pre = block->stmts[i];
                    }
                    if (pre != NULL)
                    {
                        Definition def;
                        def.block = block;
                        def.stmt = pre;
                        def.def_var = pre->get_def_var();
                        stmt->use_def.push_back(def);
                    }
                    else
                    {
                        for (auto def : block->def_in)
                        {
                            if (def.def_var == use_var)
                            {
                                stmt->use_def.push_back(def);
                            }
                        }
                    }
                }
            }
        }
        /*
        for (auto pair : basic_blocks)
        {
            BasicBlock *block = pair.second;
            for (IrStmt *stmt : block->stmts)
            {
                if (stmt == NULL)
                    continue;

                for (auto def : stmt->use_def)
                {
                    std::cout << def.stmt->to_string() << '\n';
                }
            }
        }*/
    }

    void find_dom_node()
    {
        std::set<BasicBlock *> U_block;

        for (auto pair : basic_blocks)
            U_block.insert(pair.second);

        for (auto pair : basic_blocks)
            if (pair.second != entry)
                pair.second->dom_node_out = U_block;

        bool is_changed = true;
        while (is_changed)
        {
            is_changed = false;
            for (auto pair : basic_blocks)
            {
                BasicBlock *block = pair.second;
                if (block == entry)
                    continue;
                block->dom_node_in = (*block->predecessor.begin())->dom_node_out;
                for (BasicBlock *pre : block->predecessor)
                {
                    std::set<BasicBlock *> tmp;
                    std::set_intersection(block->dom_node_in.begin(), block->dom_node_in.end(),
                                          pre->dom_node_out.begin(), pre->dom_node_out.end(),
                                          std::inserter(tmp, tmp.begin()));
                    block->dom_node_in = tmp;
                }

                std::set<BasicBlock *> tmp_out;
                tmp_out = block->dom_node_in;
                tmp_out.insert(block);
                if (!equal(tmp_out, block->dom_node_out))
                {
                    is_changed = true;
                    block->dom_node_out = tmp_out;
                }
            }
        }
    }

    void find_back_edge()
    {
        for (auto pair : basic_blocks)
        {
            BasicBlock *block = pair.second;
            for (BasicBlock *dom_node : block->dom_node_in)
            {
                if (find(block->successor, dom_node))
                {
                    back_edge.push_back(std::make_pair(block, dom_node));
                }
            }
        }
    }

    void find_natural_loops()
    {
        for (auto pair : back_edge)
        {
            std::vector<BasicBlock *> stack;
            stack.resize(100);
            BasicBlock *first = pair.first;
            BasicBlock *second = pair.second;
            int top = 0;
            stack[top++] = first;
            NaturalLoop natural_loop;
            natural_loop.nodes.push_back(first);
            natural_loop.nodes.push_back(second);
            while (top)
            {
                BasicBlock *cur_node = stack[--top];
                for (BasicBlock *pre : cur_node->predecessor)
                {
                    if (!find(natural_loop.nodes, pre))
                    {
                        natural_loop.nodes.push_back(pre);
                        stack[top++] = pre;
                    }
                }
            }
            natural_loops.push_back(natural_loop);
        }
    }

    void find_loop_invariant(NaturalLoop &loop)
    {
        auto &nodes = loop.nodes;
        auto &invariant = loop.invariant;
        bool is_changed = true;
        //while (is_changed)
        {
            is_changed = false;
            for (auto node : nodes)
            {
                for (auto stmt : node->stmts)
                {
                    if (stmt == NULL || find(invariant, std::make_pair(node, stmt)) ||
                        stmt->type == IR_TYPE_GOTO || stmt->type == IR_TYPE_FUNC_CALL ||
                        stmt->type == IR_TYPE_IF || stmt->type == IR_TYPE_RETURN ||
                        stmt->type == IR_TYPE_OTHER)
                        continue;
                    bool is_def_outside = true;
                    for (auto def : stmt->use_def)
                    {
                        /*if (find(invariant, std::make_pair(def.block, def.stmt)))
                        {
                            continue;
                        }*/
                        if (find(nodes, def.block))
                        {
                            is_def_outside = false;
                            break;
                        }
                    }
                    if (is_def_outside)
                    {
                        invariant.push_back(std::make_pair(node, stmt));
                        is_changed = true;
                    }
                }
            }
        }
    }

    void move_loop_invariant()
    {
        find_dom_node();
        find_back_edge();
        find_natural_loops();
        for (NaturalLoop &loop : natural_loops)
        {
            //print_loop(loop);
            find_loop_invariant(loop);
            //print_loop_invariant(loop);
            move_loop_invariant(loop);
        }
    }

    void move_loop_invariant(NaturalLoop &loop)
    {
        static int new_label = 0;
        BasicBlock *head = loop.nodes[1];
        std::vector<BasicBlock *> pre_node_outside_loop;
        for (BasicBlock *pre : head->predecessor)
            if (!find(loop.nodes, pre))
                pre_node_outside_loop.push_back(pre);

        if (pre_node_outside_loop.size() == 1)
        {
            BasicBlock *pre_node = pre_node_outside_loop[0];
            IrStmt *last_stmt = pre_node->get_last_stmt();
            if (last_stmt->type != IR_TYPE_GOTO && last_stmt->type != IR_TYPE_IF)
            {
                for (auto pair : loop.invariant)
                {
                    BasicBlock *block = pair.first;
                    for (int i = 0; i < block->stmts.size(); ++i)
                    {
                        if (block->stmts[i] == pair.second &&
                            (block->stmts[i]->type != IR_TYPE_ASSIGN || find(block->stmts[i]->r_addr, "[")))
                        {
                            IrStmt *new_stmt = new IrStmt(*block->stmts[i]);
                            new_stmt->target = std::string("%00") + std::to_string(new_label++);
                            block->stmts[i]->type = IR_TYPE_ASSIGN;
                            block->stmts[i]->r_addr = new_stmt->target;
                            pre_node->stmts.push_back(new_stmt);
                        }
                    }
                }
            }
        }
        /*
        return;
        std::vector<std::pair<BasicBlock *, IrStmt *>> removable_stmt;
        for (auto &pair : loop.invariant)
            if (is_stmt_removable(loop, pair))
                removable_stmt.push_back(pair);
        if (removable_stmt.size() == 0)
            return;

        for (auto pair : removable_stmt)
            std::cout << pair.second->to_string() << '\n';
        std::cout << '\n';

        for (auto pair : removable_stmt)
        {
            BasicBlock *block = pair.first;
            for (int i = 0; i < block->stmts.size(); ++i)
            {
                if (block->stmts[i] == pair.second)
                {
                    block->stmts[i] = NULL;
                }
            }
        }

        BasicBlock *head = loop.nodes[1];
        std::vector<BasicBlock *> pre_node_outside_loop;
        for (BasicBlock *pre : head->predecessor)
            if (!find(loop.nodes, pre))
                pre_node_outside_loop.push_back(pre);

        if (pre_node_outside_loop.size() == 1)
        {
            BasicBlock *pre_node = pre_node_outside_loop[0];
            IrStmt *last_stmt = pre_node->get_last_stmt();
            if (last_stmt->type != IR_TYPE_GOTO && last_stmt->type != IR_TYPE_IF)
            {
                for (auto pair : removable_stmt)
                {
                    pre_node->stmts.push_back(pair.second);
                }
            }
        }
        BasicBlock *new_block = new BasicBlock();
        new_block->label = get_next_label();
        //insert_block(head, new_block);*/
    }

    bool is_stmt_removable(NaturalLoop &loop, std::pair<BasicBlock *, IrStmt *> &pair)
    {
        BasicBlock *block = pair.first;
        IrStmt *stmt = pair.second;

        std::vector<BasicBlock *> out_nodes;
        for (BasicBlock *node : loop.nodes)
        {
            for (BasicBlock *next : node->successor)
            {
                if (!find(loop.nodes, next))
                {
                    out_nodes.push_back(node);
                }
            }
        }

        for (BasicBlock *node : out_nodes)
        {
            if (!find(node->dom_node_in, block))
            {
                return false;
            }
        }

        std::string def_var = stmt->get_def_var();
        for (BasicBlock *node : loop.nodes)
        {
            for (IrStmt *other_stmt : node->stmts)
            {
                if (other_stmt == NULL || other_stmt == stmt)
                    continue;
                if (other_stmt->get_def_var() == def_var)
                {
                    return false;
                }
            }
        }

        for (BasicBlock *node : loop.nodes)
        {
            for (IrStmt *other_stmt : node->stmts)
            {
                if (other_stmt == NULL)
                    continue;

                if (other_stmt->exist_use(def_var))
                {
                    auto def_list = other_stmt->get_def(def_var);
                    for (auto def : def_list)
                    {
                        if (def.stmt != stmt)
                        {
                            return false;
                        }
                    }
                }
            }
        }

        return true;
    }

    void global_optimize()
    { 
        usable_expr_analyse();
        delete_common_subexpr();
        delete_copy_spread();

        for (int i = 0; i < 6; ++i)
        {
            clear();
            active_var_analyse();
            optimize_basic_blocks();
            usable_expr_analyse();
            delete_common_subexpr();
            delete_copy_spread();
        }
        
        //std::cout << to_string();

        arrival_definition_analyze();
        move_loop_invariant();

        for (int i = 0; i < 2; ++i)
        {
            clear();
            active_var_analyse();
            optimize_basic_blocks();
            usable_expr_analyse();
            delete_common_subexpr();
            delete_copy_spread();
        }
/*
        arrival_definition_analyze();
        move_loop_invariant();

        for (int i = 0; i < 2; ++i)
        {
            clear();
            active_var_analyse();
            optimize_basic_blocks();
            usable_expr_analyse();
            delete_common_subexpr();
            delete_copy_spread();
        }

        clear();
        arrival_definition_analyze();
        move_loop_invariant();

        clear();
        active_var_analyse();
        optimize_basic_blocks();
        usable_expr_analyse();
        delete_common_subexpr();
        delete_copy_spread();*/
    }

    std::string get_name()
    {
        auto vec = str_split(decls[0], " ");
        return std::string(vec[1].begin(), vec[1].begin() + vec[1].find('('));
    }

    bool is_reducible()
    {
        if (find(decls[0], "["))
            return false;
        for (auto pair : basic_blocks)
        {
            for (auto stmt : pair.second->stmts)
            {
                if (find(stmt->l_addr, "@") || find(stmt->r_addr, "@") || find(stmt->target, "@"))
                    return false;

                if (stmt->type == IR_TYPE_FUNC_CALL && !find(reducible_func_list, stmt->func_name))
                    return false;

                if (stmt->type == IR_TYPE_RETURN && stmt->target == "")
                    return false;
            }
        }
        return true;
    }

    void print_loop(NaturalLoop &loop)
    {
        for (auto block : loop.nodes)
        {
            if (block->has_label)
                std::cout << "label " << block->label << "\n";
            for (auto stmt : block->stmts)
                if (stmt != NULL)
                    std::cout << stmt->to_string() << "\n";
        }
        std::cout << '\n';
    }

    void print_loop_invariant(NaturalLoop &loop)
    {
        for (auto pair : loop.invariant)
        {
            std::cout << pair.second->to_string() << '\n';
        }
        std::cout << '\n';
    }

    void print_use_def_chain()
    {
        for (auto pair : basic_blocks)
        {
            for (IrStmt *stmt : pair.second->stmts)
            {
                for (auto def : stmt->use_def)
                {
                    std::cout << def.stmt->to_string() << '\n';
                }
            }
        }
    }

    void print_active_var()
    {
        for (auto pair : basic_blocks)
        {
            std::cout << pair.first << ":\n";
            for (auto var : pair.second->active_var_out)
            {
                std::cout << var << "\n";
            }
            std::cout << "\n";
        }
    }

    void print_usable_expr()
    {
        for (auto pair : basic_blocks)
        {
            std::cout << pair.first << ":\n";
            for (auto expr : pair.second->expr_in)
            {
                std::cout << expr.target + " " + expr.l_addr + expr.op + expr.r_addr << '\n';
            }
            std::cout << "\n";
        }
    }

private:
    std::set<int> builded_basic_block;
    BasicBlock *build_basic_block(int begin, bool has_label)
    { //static int sss=0;
        //printf("%d\n",sss++);
        //printf("jjj\n");
        if (builded_basic_block.find(first_stmt[begin]) != builded_basic_block.end())
        {
            //printf("111\n");
            BasicBlock *block = basic_blocks[first_stmt[begin]];
            /*printf("222\n");
            printf("%x\n",block);
            printf("%d\n",!block->has_label);*/
            if (!block->has_label)
            {                            //printf("333\n");
                block->has_label = true; //printf("444\n");
                block->label = get_next_label();
            }
            //printf("555\n");
            return block;
        }
        //printf("BBB\n");
        builded_basic_block.insert(first_stmt[begin]);

        BasicBlock *basic_block = new BasicBlock();
        basic_block->has_label = has_label;
        if (has_label)
            basic_block->label = get_next_label();
        basic_blocks[first_stmt[begin]] = basic_block;
        //printf("CCC\n");
        int next; //the next first stmt
        next = body.size();
        //printf("stmt_top=%d\n",stmt_top);
        for (int iter = 0; iter < stmt_top; ++iter)
        {
            if (iter == begin)
            {
                ++iter;
                //printf("+++++++iter=%d\n",iter);
                //printf("+++++++stmt_top=%d\n",stmt_top);
                //printf("first_stmt[iter]=%d\n",first_stmt[iter]);
                if (iter < stmt_top)
                    next = first_stmt[iter];
                //printf("next=%d\n",next);
                break;
            }
        }
        //printf("DDD\n");

        /*        int next; //the next first stmt
        ++begin;
        if (begin == first_stmt.end())
            next = body.size();
        else
            next = first_stmt[begin];
        --begin;*/

        int last_stmt = -1; //the last stmt of this basic block
        //printf("first_stmt[begin]=%d\n",first_stmt[begin]);
        //printf("next=%d\n",next);
        for (int i = first_stmt[begin]; i < next && i < body.size(); i++)
        {
            //std::cout<<"body[i]="<<body[i]<<'\n';
            if (body[i] != "" && str_split(body[i], " ")[0] != "label") //ignore all labels and blanks
            {
                basic_block->add(body[i]);
                last_stmt = i;
                //printf("i=%d\n",i);
            }
        }
        if (last_stmt == -1)
        {
            last_stmt = next - 1;
        }
        //printf("EEE\n");
        /*
        if (last_stmt >= body.size())
            last_stmt = body.size() - 1;*/

        if (str_split(body[last_stmt], " ")[0] == "return")
        {
            basic_block->successor.push_back(exit);
            exit->predecessor.push_front(basic_block);
        } /*
printf("FFF\n");
printf("FFF_begin=%d\n",begin);
printf("FFF_last_stmt=%d\n",last_stmt);
std::cout<<body[last_stmt]<<'\n';*/
        if (++begin < stmt_top && str_split(body[last_stmt], " ")[0] != "goto")
        {
            //printf("begin:%d\n",begin);
            BasicBlock *_next = build_basic_block(begin, false);
            //printf("kkk\n");
            _next->predecessor.push_back(basic_block);
            //printf("oooo\n");
            basic_block->successor.push_back(_next);
            //printf("uuuu\n");
        }
        //printf("GGG\n");
        //printf("45678  %d\n",last_stmt);
        if (unresolved_jump.find(last_stmt) != unresolved_jump.end())
        { //printf("123123  %d\n",last_stmt);
            int n = atoi(unresolved_jump[last_stmt].c_str());
            int i;
            for (i = 0; i < stmt_top; ++i)
                if (first_stmt[i] == n)
                    break;
            //printf("begin:%d\n",begin);
            BasicBlock *_next = build_basic_block(i, true);
            IrStmt *jump_stmt = *(basic_block->stmts.rbegin());
            jump_stmt->target = _next->label;
            _next->predecessor.push_back(basic_block);
            basic_block->successor.push_back(_next);
        }
        return basic_block;
    }
};
void ir_optimize();

#endif //IR_OPTIMIZE_H