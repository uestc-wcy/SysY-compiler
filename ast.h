#ifndef __AST_H
#define __AST_H

extern int yylineno;
extern char *yytext;

#include <string.h>
#include <string>
#include <vector>

enum
{
	AST_COMP_UNIT,
	AST_CONST_DECL,
	AST_VAR_DECL,
	AST_FUNC_DEF,
	AST_FUNC_CALL,
	AST_CONST_NUM_DEF,
	AST_CONST_ARRAY_DEF,
	AST_CONST_EXP,
	AST_CONST_INIT_VAL,
	AST_VAR_NUM_DEF,
	AST_VAR_ARRAY_DEF,
	AST_INIT_VAL,
	AST_FUNC_FPARAM,
	AST_ASSIGN_STMT,
	AST_EXP_STMT,
	AST_IF_STMT,
	AST_WHILE_STMT,
	AST_BREAK_STMT,
	AST_CONTINUE_STMT,
	AST_RETURN_STMT,
	AST_BLOCK_STMT,
	AST_OP_BINARY,
	AST_OP_UNARY,
	AST_INT_CONST,
	AST_LIST,
	AST_LVAL_NUM,
	AST_LVAL_ARRAY,
	AST_STR_CONST
};

enum
{
	BTYPE_INT,
	FUNCTYPE_INT,
	FUNCTYPE_VOID
};

struct Node
{
	int type;
	struct Node *content_for_list;
	union {
		int ival;		  // int
		std::string sval; 

		struct Node *list;

		struct // function def or function call
		{
			char *fname;
			int ftype;
			struct Node *args;
			struct Node *body;
		} func;

		struct // Declaration
		{
			int btype;
			struct Node *defList;
		} decl;

		struct // function args
		{
			int btype;
			bool isArray;
			char *name;
			struct Node *expList;
		} funcArg;

		struct // Binary operator
		{
			char *op;
			struct Node *left;
			struct Node *right;
		} binary;

		struct // Unary operator
		{
			char op;
			struct Node *operand;
		} unary;

		struct // var def or const def
		{
			char *name;
			struct Node *initVal;
		} num;

		struct // array def
		{
			char *name;
			struct Node *expList;
			struct Node *initval;
		} array;

		struct // If statement
		{
			struct Node *cond;
			struct Node *then;
			struct Node *els;
		} ifStmt;

		struct // While statement
		{
			struct Node *cond;
			struct Node *block;
		} whileStmt;

		struct Node *retval; // Return statement
	};
};

typedef Node *pnode;

struct Node *newString(std::string str_const);
void showAST(struct Node *list, int depth);
void showAST2(struct Node *list, int depth);
void *checkedMalloc(int len);
struct Node *newNode();
struct Node *newList(struct Node *list, struct Node *node);
struct Node *newNum(int num);
struct Node *newIfStmt(struct Node *cond, struct Node *trueStmt, struct Node *falsestmt);
struct Node *newWhileStmt(struct Node *cond, struct Node *block);
struct Node *newDecl(bool isConst, struct Node *defList);
struct Node *newNumDef(bool isConst, const char *name, struct Node *initVal);
struct Node *newArrayDef(bool isConst, const char *name, struct Node *expList, struct Node *initVal);
struct Node *newFunc(const char *type, const char *fname, struct Node *args, struct Node *block);
struct Node *newFuncFParam(const char *type, const char *name, bool isArray, struct Node *expList);
struct Node *newBinary(const char *op, struct Node *left, struct Node *right);
struct Node *newUnary(const char op, struct Node *operand);
struct Node *newSimpleNode(int type);
struct Node *newReturnStmt(struct Node *retVal);
struct Node *newLVal(const char *name, struct Node *expList);
#endif //__AST_H
