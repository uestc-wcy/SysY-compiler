%{
#include<stdio.h>
#include<string.h>
#include"ast.h"
#include"gen_code.h"
#include"gen_arm.h"
#include"ir_optimize.h"


int yylex(void);
void yyerror(char*); 
struct Node *astHEAD; 

extern bool is_in_opt_mode=false;
%}

%token INT
%token VOID
%token IF
%token ELSE
%token WHILE
%token RETURN
%token BREAK
%token CONST
%token CONTINUE
%token EOP

%token<iValue> IntConst
%token<sValue> Ident CMP JUDGE LOGIC_AND LOGIC_OR StringConst

%union
{
	int iValue;
        char cValue;
	char *sValue;
	struct Node* pNode;
}

%type <pNode> 
CompUnit Decl FuncDef 
ConstDecl VarDecl
ConstDef
ConstExp ConstInitVal
VarDef InitVal
Exp
FuncFParams Block
FuncFParam
BlockItem
Stmt
LVal Cond
AddExp LOrExp
Number
UnaryExp PrimaryExp FuncRParams
MulExp
RelExp EqExp
LAndExp
ConstDef_list
ConstExp_list
ConstInitval_list
VarDef_list
Initval_list
Exp_list
Exp_list2
BlockItem_list
%type<sValue>Type 
%type<cValue>UnaryOp

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE


%%
CompUnit: 
       Decl {$$=newList(NULL,$1); astHEAD=$$;}
       | FuncDef {$$=newList(NULL,$1);astHEAD=$$;}
       | CompUnit Decl {$$=newList($1,$2);}
       | CompUnit FuncDef {$$=newList($1,$2);}
       | CompUnit EOP { 
       //showAST(astHEAD,0);
       gen_code(astHEAD);
       ir_optimize();
       return 0;}
       ;

Decl:
        ConstDecl {$$=$1;}
        |VarDecl  {$$=$1;}
        ;

ConstDecl: 
        CONST Type ConstDef_list ';'  {$$=newDecl(true,$3);}
        ;

ConstDef_list:
        ConstDef  {$$=newList(NULL,$1);}
        |ConstDef_list ',' ConstDef  {$$=newList($1,$3);}
        ;


ConstDef:
        Ident '=' ConstInitVal  {$$=newNumDef(true,$1,$3);}
        |Ident ConstExp_list '=' ConstInitVal  {$$=newArrayDef(true,$1,$2,$4);}
        ;

ConstExp_list:
        '[' ConstExp ']'  {$$=newList(NULL,$2);}
        |ConstExp_list '[' ConstExp ']'  {$$=newList($1,$3);}
        ;

ConstInitVal:
        ConstExp {$$=$1;}
        |'{' '}'  {$$=newList(NULL,NULL);}
        |'{' ConstInitval_list '}'  {$$=$2;}
        ;

ConstInitval_list:
        ConstInitVal  {$$=newList(NULL,$1);}
        |ConstInitval_list ',' ConstInitVal  {$$=newList($1,$3);}
        ;

VarDecl:
        Type VarDef_list ';'  {$$=newDecl(false,$2);}
        ;

VarDef_list:
        VarDef  {$$=newList(NULL,$1);}
        |VarDef_list ',' VarDef  {$$=newList($1,$3);}
        ;

VarDef:
        Ident   {$$=newNumDef(false,$1,NULL);}
        |Ident '=' InitVal  {$$=newNumDef(false,$1,$3);}
        |Ident ConstExp_list  {$$=newArrayDef(false,$1,$2,NULL);}
        |Ident ConstExp_list '=' InitVal  {$$=newArrayDef(false,$1,$2,$4);}  
        ;

InitVal:
        Exp {$$=$1;}
        |'{' '}'  {$$=newList(NULL,NULL);}
        |'{' Initval_list '}'  {$$=$2;}
        ;

Initval_list:
        InitVal  {$$=newList(NULL,$1);}
        |Initval_list ',' InitVal  {$$=newList($1,$3);}
        ;

FuncDef:
        Type Ident '(' ')' Block {$$=newFunc($1,$2,NULL,$5);}
        |Type Ident '(' FuncFParams ')' Block  {$$=newFunc($1,$2,$4,$6);}
        ;

Type:
        INT  {$$="int";}
        |VOID  {$$="void";}
        ;

FuncFParams:
        FuncFParam  {$$=newList(NULL,$1);}
        |FuncFParams ',' FuncFParam  {$$=newList($1,$3);}
        ;

FuncFParam:
        Type Ident  {$$=newFuncFParam($1,$2,false,NULL);}
        |Type Ident '[' ']'  {$$=newFuncFParam($1,$2,true,NULL);}
        |Type Ident '[' ']' Exp_list  {$$=newFuncFParam($1,$2,true,$5);}
        ;

Exp_list:
        '[' Exp ']'  {$$=newList(NULL,$2);}
        |Exp_list '[' Exp ']' {$$=newList($1,$3);}
        ;

Block:
        '{' '}'  {$$=NULL;}
        |'{' BlockItem_list '}'  {$$=$2;}
        ;

BlockItem_list:
        BlockItem  {$$=newList(NULL,$1);}
        |BlockItem_list BlockItem {$$=newList($1,$2);}
        ;

BlockItem:
        Decl  {$$=$1;}
        |Stmt  {$$=$1;}
        ;

Stmt:
        LVal '=' Exp ';'  {$$=newBinary("=",$1,$3);}
        |';'  {$$=NULL;}
        |Exp ';'  {$$=$1;}
        |Block  {$$=$1;}
        |IF '(' Cond ')' Stmt {$$=newIfStmt($3,$5,NULL);}
        |IF '(' Cond ')' Stmt ELSE Stmt {$$=newIfStmt($3,$5,$7);}
        |WHILE '(' Cond ')' Stmt  {$$=newWhileStmt($3,$5);}
        |BREAK ';'  {$$=newSimpleNode(AST_BREAK_STMT);}
        |CONTINUE ';'  {$$=newSimpleNode(AST_CONTINUE_STMT);}
        |RETURN ';'  {$$=newReturnStmt(NULL);}
        |RETURN Exp ';'  {$$=newReturnStmt($2);}
        ;

Exp:
        AddExp  {$$=$1;}
        ;

Cond:
        LOrExp  {$$=$1;}
        ;

LVal:
        Ident  {$$=newLVal($1,NULL);}
        |Ident Exp_list  {$$=newLVal($1,$2);}
        ;

PrimaryExp:
        '(' Exp ')'  {$$=$2;}
        |LVal  {$$=$1;}
        |Number  {$$=$1;}
        |StringConst {$$=newString($1);}
        ;

Number:
        IntConst  {$$=newNum($1);}
        ;

UnaryExp:
        PrimaryExp  {$$=$1;}
        |Ident '(' ')'  {$$=newFunc(NULL,$1,NULL,NULL);}
        |Ident '(' FuncRParams ')'  {$$=newFunc(NULL,$1,$3,NULL);}
        |UnaryOp UnaryExp  {$$=newUnary($1,$2);}
        ;

UnaryOp:
        '+'  {$$='+';}
        |'-'  {$$='-';}
        |'!'  {$$='!';}
        ;

FuncRParams: /*����������һ���ڵ�Exp_list2*/
        Exp_list2  {$$=$1;}
        ;

Exp_list2:
        Exp  {$$=newList(NULL,$1);}
        |Exp_list2 ',' Exp {$$=newList($1,$3);}
        ;

MulExp:
        UnaryExp  {$$=$1;}
        |MulExp '*' UnaryExp  {$$=newBinary("*",$1,$3);}
        |MulExp '/' UnaryExp  {$$=newBinary("/",$1,$3);}
        |MulExp '%' UnaryExp  {$$=newBinary("%",$1,$3);}
        ;

AddExp:
        MulExp  {$$=$1;}
        |AddExp '+' MulExp  {$$=newBinary("+",$1,$3);}
        |AddExp '-' MulExp  {$$=newBinary("-",$1,$3);}
        ;

RelExp:
        AddExp  {$$=$1;}
        |RelExp CMP AddExp  {$$=newBinary($2,$1,$3);}
        ;

EqExp:
        RelExp  {$$=$1;}
        |EqExp JUDGE RelExp  {$$=newBinary($2,$1,$3);}
        ;

LAndExp:
        EqExp  {$$=$1;}
        |LAndExp LOGIC_AND EqExp  {$$=newBinary($2,$1,$3);}
        ;

LOrExp:
        LAndExp  {$$=$1;}
        |LOrExp LOGIC_OR LAndExp  {$$=newBinary($2,$1,$3);}
        ;

ConstExp:
        AddExp  {$$=$1;}
        ;


       
%%
void yyerror(char *s)
{
	fprintf(stderr,"error:%s\nline:%d\nyytext:%s",yytext,yylineno,yytext);
   
}

int main(int argc, char* argv[])
{
        
        
        yyparse();

        
	return 0;

}
