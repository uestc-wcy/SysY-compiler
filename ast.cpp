#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include <iostream>
#include "ast.h"
using namespace std;

void *checkedMalloc(int len)
{
    void *p = malloc(len);
    if (p == NULL)
    {
        printf("Run out of memory\n");
        exit(1);
    }
    return p;
}

void showAST(struct Node *list, int depth)
{
    if (list == NULL)
        return;

    printf("\n");
    for (int i = 0; i < depth; i++)
    {
        printf("\t");
    }

    switch (list->type)
    {
    case AST_LIST:
        printf("%s", "ast_list");
        showAST(list->content_for_list, depth + 1);
        showAST(list->list, depth);
        break;

    case AST_INT_CONST:
        printf("%s:%d", "IntConst", list->ival);
        break;

    case AST_IF_STMT:
        printf("%s", "IFStmt,and next three block is cond,then,els");
        showAST(list->ifStmt.cond, depth + 1);
        showAST(list->ifStmt.then, depth + 1);
        showAST(list->ifStmt.els, depth + 1);
        break;

    case AST_WHILE_STMT:
        printf("%s", "WhileStmt, and next two block is cond,block");
        showAST(list->whileStmt.cond, depth + 1);
        showAST(list->whileStmt.block, depth + 1);
        break;

    case AST_CONST_DECL:
        printf("%s", "Const DECL(int)");
        showAST(list->decl.defList, depth + 1);
        break;

    case AST_VAR_DECL:
        printf("%s", "Var DECL(int)");
        showAST(list->decl.defList, depth + 1);
        break;

    case AST_CONST_NUM_DEF:
        printf("%s,%s", "Const num DECL(int),name:", list->num.name);
        showAST(list->num.initVal, depth + 1);
        break;

    case AST_VAR_NUM_DEF:
        printf("%s,%s", "Var num DECL(int),name:", list->num.name);
        showAST(list->num.initVal, depth + 1);
        break;

    case AST_CONST_ARRAY_DEF:
        printf("%s,%s", "Const array DECL(int),name:", list->num.name);
        showAST(list->array.expList, depth + 1);
        showAST(list->array.initval, depth + 1);
        break;

    case AST_VAR_ARRAY_DEF:
        printf("%s,%s", "Var array DECL(int),name:", list->num.name);
        showAST(list->array.expList, depth + 1);
        showAST(list->array.initval, depth + 1);
        break;

    case AST_FUNC_CALL:
        printf("%s,%s", "Function call,name:", list->func.fname);
        showAST(list->func.args, depth + 1);
        showAST(list->func.body, depth + 1);
        break;

    case AST_FUNC_DEF:
        printf("%s,%s", "Function def,name:", list->func.fname);
        if (list->func.ftype == FUNCTYPE_INT)
        {
            printf("return type: int");
        }
        else if (list->func.ftype == FUNCTYPE_VOID)
        {
            printf("return type: void");
        }
        showAST(list->func.args, depth + 1);
        showAST(list->func.body, depth + 1);
        break;

    case AST_FUNC_FPARAM:
        printf("%s,%s", "params of function,name:", list->funcArg.name);
        if (list->funcArg.isArray)
            printf("(Params type is array)");

        showAST(list->funcArg.expList, depth + 1);
        break;

    case AST_OP_BINARY:
        printf("%s,%s", "binary opera,op:", list->binary.op);
        showAST(list->binary.left, depth + 1);
        showAST(list->binary.right, depth + 1);
        break;

    case AST_OP_UNARY:
        printf("%s,%c", "unary opera,op:", list->unary.op);
        showAST(list->unary.operand, depth + 1);
        break;

    case AST_BREAK_STMT:
        printf("%s", "break");
        break;

    case AST_CONTINUE_STMT:
        printf("%s", "continue");
        break;

    case AST_RETURN_STMT:
        printf("%s", "return stmt");
        if (list->retval != NULL)
        {
            printf("(with expr)");
        }
        showAST(list->retval, depth + 1);
        break;

    case AST_LVAL_NUM:
        printf("%s,%s", "lval num,name:", list->num.name);
        break;

    case AST_LVAL_ARRAY:
        printf("%s,%s", "lval array,name:", list->array.name);
        showAST(list->array.expList, depth + 1);
        break;

    case AST_STR_CONST:
        cout << list->sval;
        break;

    default:
        break;
    }
    return;
}

struct Node *newNode()
{
    struct Node *p = (struct Node *)checkedMalloc(sizeof(struct Node));
    memset(p, 0, sizeof(struct Node));
    p->list = NULL;
    return p;
}

struct Node *newList(struct Node *list, struct Node *node)
{

    if (list == NULL)
    {
        list = newNode();
        list->type = AST_LIST;
        list->content_for_list = node;
        return list;
    }

    struct Node *list_tem = newNode();
    list_tem->type = AST_LIST;
    list_tem->content_for_list = node;

    struct Node *listTail = list;

    while (listTail->list != NULL)
    {
        listTail = listTail->list;
    }
    listTail->list = list_tem;
    return list;
}

struct Node *newNum(int num)
{
    struct Node *p = newNode();
    p->ival = num;
    p->type = AST_INT_CONST;
    return p;
}
struct Node *newString(std::string str_const)
{
    struct Node *p = newNode();
    p->sval = str_const;
    p->type = AST_STR_CONST;
    return p;
}

struct Node *newIfStmt(struct Node *cond, struct Node *then, struct Node *els)
{
    struct Node *p = newNode();
    p->type = AST_IF_STMT;
    p->ifStmt.cond = cond;
    p->ifStmt.then = then;
    p->ifStmt.els = els;
    return p;
}

struct Node *newWhileStmt(struct Node *cond, struct Node *block)
{
    struct Node *p = newNode();
    p->type = AST_WHILE_STMT;
    p->whileStmt.cond = cond;
    p->whileStmt.block = block;
    return p;
}

struct Node *newDecl(bool isConst, struct Node *defList)
{
    struct Node *p = newNode();
    p->type = isConst ? AST_CONST_DECL : AST_VAR_DECL;
    p->decl.btype = BTYPE_INT;
    p->decl.defList = defList;
    return p;
}

struct Node *newNumDef(bool isConst, const char *name, struct Node *initVal)
{
    struct Node *p = newNode();
    p->type = isConst ? AST_CONST_NUM_DEF : AST_VAR_NUM_DEF;
    p->num.name = strdup(name);
    p->num.initVal = initVal;
    return p;
}

struct Node *newArrayDef(bool isConst, const char *name, struct Node *expList, struct Node *initVal)
{
    struct Node *p = newNode();
    p->type = isConst ? AST_CONST_ARRAY_DEF : AST_VAR_ARRAY_DEF;
    p->array.name = strdup(name);
    p->array.initval = initVal;
    p->array.expList = expList;
    return p;
}
struct Node *newFunc(const char *type, const char *fname, struct Node *args, struct Node *body)
{
    struct Node *p = newNode();
    p->type = (type == NULL) ? AST_FUNC_CALL : AST_FUNC_DEF;
    if (type == NULL)
    {
        p->func.fname = strdup(fname);
        p->func.args = args;
        return p;
    }
    else
    {
        if (strcmp(type, "int") == 0)
            p->func.ftype = FUNCTYPE_INT;
        else if (strcmp(type, "void") == 0)
            p->func.ftype = FUNCTYPE_VOID;
        p->func.fname = strdup(fname);
        p->func.args = args;
        p->func.body = body;
        return p;
    }
}

struct Node *newFuncFParam(const char *type, const char *name, bool isArray, struct Node *expList)
{
    struct Node *p = newNode();
    p->type = AST_FUNC_FPARAM;
    p->funcArg.btype = BTYPE_INT;
    p->funcArg.isArray = isArray;
    p->funcArg.name = strdup(name);
    p->funcArg.expList = expList;
    return p;
}
struct Node *newBinary(const char *op, struct Node *left, struct Node *right)
{
    struct Node *p = newNode();
    if (!strcmp(op,"="))
        p->type = AST_ASSIGN_STMT;
    else
        p->type = AST_OP_BINARY;
    p->binary.op = strdup(op);
    p->binary.left = left;
    p->binary.right = right;
    return p;
}
struct Node *newUnary(const char op, struct Node *operand)
{
    struct Node *p = newNode();
    p->type = AST_OP_UNARY;
    p->unary.op = op;
    p->unary.operand = operand;
    return p;
}
struct Node *newSimpleNode(int type)
{
    struct Node *p = newNode();
    p->type = type;
    return p;
}
struct Node *newReturnStmt(struct Node *retVal)
{
    struct Node *p = newNode();
    p->type = AST_RETURN_STMT;
    p->retval = retVal;
    return p;
}
struct Node *newLVal(const char *name, struct Node *expList)
{
    struct Node *p = newNode();
    if (expList == NULL)
    {
        p->num.name = strdup(name);
        p->type = AST_LVAL_NUM;
    }
    else
    {
        p->array.name = strdup(name);
        p->array.expList = expList;
        p->type = AST_LVAL_ARRAY;
    }
    return p;
}
