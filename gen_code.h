#ifndef GEN_CODE_H
#define GEN_CODE_H

#include"ast.h"

void print_ircode(Node *tree);
void gen_code(Node *tree);
void gen_code_global_decl(Node*tree);
void gen_code_func_def(Node*tree);
void gen_code_func_fparam(Node*tree);
void gen_code_func_body(Node *tree);
void gen_code_block(std::stringstream &decl_buffer,std::stringstream &stmt_buffer,Node *body);
void gen_code_local_decl(std::stringstream &decl_buffer,std::stringstream &stmt_buffer,Node*tree);
//void gen_code_array_init(std::stringstream &stmt_buffer, Node *tree, LocalSymbol *symbol, const std::string name, int begin, int length, int dim, int depth);
std::string gen_code_expr(std::stringstream &stmt_buffer,Node*tree);
std::string gen_code_expr_array(std::stringstream &stmt_buffer,Node*tree);
std::string gen_code_expr_func_call(std::stringstream &stmt_buffer,Node*tree);
std::vector<std::string> gen_code_func_Rparams(std::stringstream &stmt_buffer, Node*tree);
std::string gen_code_func_Rparams_array(std::stringstream &stmt_buffer, Node*tree);
void gen_code_assign_expr(std::stringstream &stmt_buffer, Node *tree);
std::string gen_code_lval(std::stringstream &stmt_buffer, Node *tree);
void gen_code_if_stmt(std::stringstream &decl_buffer,std::stringstream &stmt_buffer,Node *tree);
void gen_code_while_stmt(std::stringstream &decl_buffer,std::stringstream &stmt_buffer,Node *tree);
void gen_code_cond(std::stringstream &stmt_buffer,Node *cond,const std::string &true_label,const std::string &false_label);
void gen_code_rel_expr(std::stringstream &stmt_buffer,Node *expr,const std::string &true_label,const std::string &false_label);
void gen_code_break_stmt(std::stringstream &stmt_buffer, Node *tree);
void gen_code_continue_stmt(std::stringstream &stmt_buffer, Node *tree);
void gen_code_return_stmt(std::stringstream &stmt_buffer, Node *tree);

#endif//GEN_CODE_H