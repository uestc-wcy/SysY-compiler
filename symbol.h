#ifndef __SYMBOL_H
#define __SYMBOL_H
#include <vector>
#include <sstream>  
#include <string.h>
#include<string>
#include <map>
#include "ast.h"

enum SymType
{
    SYM_TYPE_CONST_INT,
    SYM_TYPE_VAR_INT,
    SYM_TYPE_CONST_ARRAY,
    SYM_TYPE_VAR_ARRAY,
    SYM_TYPE_CONST_STR,
    SYM_TYPE_FUNC_INT,
    SYM_TYPE_FUNC_VOID
};

enum FuncRParamType
{
    PARAM_INT,
    PARAM_ARRAY
};

class Symbol
{
public:
    char *name;
    int type;
    int intVal;
    std::vector<int> array_dim;
};

class GlobalSymbol : public Symbol
{
public:
    bool is_param_fixed;
    std::vector<int> func_arg_type_list;
    GlobalSymbol()
    {
        intVal=0;
        is_param_fixed=true;
    }
};

class GlobalSymTable
{
private:
    std::vector<GlobalSymbol *> symbols;

public:
    GlobalSymbol *get(const char *name)
    {
        for (GlobalSymbol *symbol : symbols)
        {
            if (strcmp(symbol->name, name) == 0)
            {
                return symbol;
            }
        }
        return NULL;
    }
    GlobalSymbol *add(const char *name, int type)
    {
        GlobalSymbol *symbol = new GlobalSymbol();
        symbol->name = strdup(name);
        symbol->type = type;
        symbols.push_back(symbol);
        return symbol;
    }
    bool setIntVal(char *name, int ival)
    {
        GlobalSymbol *symbol = get(name);
        if (symbol == NULL)
            return false;
        symbol->intVal = ival;
        return true;
    }

    bool getIntVal(char *name, int *dest)
    {
        GlobalSymbol *symbol = get(name);
        if (symbol == NULL)
            return false;
        *dest = symbol->intVal;
        return true;
    }
};

static int sum_local_sym_table=0;

class LocalSymbol : public Symbol
{
public:
    unsigned int lable; //用于区别同一函数内不同作用域的同名符号

    LocalSymbol()
    {
        intVal=0xFFFFFFFF;//undefined init value
    }

    std::string internal_name()
    {
        return std::string("%")+name+std::to_string(lable);
    }
};

class LocalSymTable
{
private:
    std::vector<LocalSymbol *> symbols;
    LocalSymTable *childTable = NULL;
    LocalSymTable *preTable = NULL;
    int lable; //独一无二的标签，用于区分不同作用域内同名的变量

    LocalSymTable *getCurrentTable()
    {
        LocalSymTable *table = this;
        while (table->childTable)
            table = table->childTable;
        return table;
    }

public:
    LocalSymbol *get(const char *name)
    {
        LocalSymTable *table = getCurrentTable();
        while (table)
        {
            for (LocalSymbol *symbol : table->symbols)
            {
                if (strcmp(symbol->name, name) == 0)
                {
                    return symbol;
                }
            }
            table = table->preTable;
        }
        return NULL;
    }
    LocalSymbol *add(char *name, int type)
    {
        LocalSymTable *table = getCurrentTable();
        LocalSymbol *symbol = new LocalSymbol();
        symbol->name = strdup(name);
        symbol->type = type;
        symbol->lable = lable;
        std::string internal_name=symbol->internal_name();
        for(LocalSymTable *p=this;p!=NULL;p=p->childTable)
        {
            for(auto s:p->symbols)
            {
                if(internal_name==s->internal_name())
                {
                    symbol->lable=hash(name, symbol->lable);
                    break;
                }
            }
        }
        table->symbols.push_back(symbol);
        return symbol;
    }

    unsigned int hash(char*s,unsigned int a)
    {
        unsigned int result=a;
        while(*s)
            result*=*(s++)*a;
        return result;
    }

    void pushTable()
    {
        LocalSymTable *currentTable = getCurrentTable();
        LocalSymTable *newTable = new LocalSymTable();
        currentTable->childTable = newTable;
        newTable->preTable = currentTable;
        newTable->lable = lable++;
    }
    void popTable()
    {
        LocalSymTable *currentTable = getCurrentTable();
        for (LocalSymbol *symbol : currentTable->symbols)
        {
            delete symbol;
        }
        if (currentTable->preTable != NULL)
        {
            currentTable = currentTable->preTable;
            delete currentTable->childTable;
            currentTable->childTable = NULL;
        }
    }
};

class ConstStringTable
{
public:
    
};

#endif //__SYMBOL_H