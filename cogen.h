#ifndef COGEN_H
#define COGEN_H

#include "parser.h"

#define NUM_REG 3   /* 利用のレジスタ数 */

/* レジスタの種類 */
typedef enum {
    ebx, esi, edi, esp, ebp, eax, ecx, edx
} reg_kind_t;

typedef struct env *env_t;
typedef struct label_gen *label_gen_t;
typedef struct stack_int *stackst_t;

struct label_gen
{
    int cnt_compound;   /* ブロックの数 */
    int cnt_label;      /* ラベルの数 */
    int cnt_return;     /* リターンの数 */
    stackst_t list_wh;     /* while ループのラベルリスト */
    int max_reg;
};

/* ラベルの生成関数 */
label_gen_t mk_label_gen();

/* 環境に関する関数群 */
env_t mk_env();
env_t set_env_params(var_decl_list_t params, env_t env);
void env_add(env_t env, var_decl_t key);
void *env_pop(env_t env);
var_decl_t env_get(env_t env, int i);
var_decl_t find(env_t env, char *key);

/* スタックに関する関数群 */
stackst_t mk_stack();
void stack_push(stackst_t l, int x);
int stack_sz(stackst_t l);
void stack_swap(stackst_t l);
int stack_pop(stackst_t l);
int stackst_top(stackst_t l);
stackst_t get_stack_register();


int cogen_program(FILE *fp, program_t p);
int cogen_fun_def(FILE *fp, fun_def_t d, label_gen_t lg);
int cogen_alloc_storage_fun_def(fun_def_t d, label_gen_t lg, int *tmp_memory);
int cogen_alloc_storage_stmt(stmt_t s, label_gen_t lg, int *save_location, int *tmp_memory);
int cogen_alloc_storage_expr(expr_t e, int *save_location, int node_tree, int *tmp_memory);
int cogen_stmt(FILE *fp, stmt_t s, env_t env, label_gen_t lg, stackst_t tmp_mem);
int cogen_stmt_empty(FILE *fp, stmt_t s, env_t env, label_gen_t lg, stackst_t tmp_mem);
int cogen_stmt_return(FILE *fp, stmt_t s, env_t env, label_gen_t lg, stackst_t tmp_mem);
int cogen_stmt_continue(FILE *fp, stmt_t s, env_t env, label_gen_t lg, stackst_t tmp_mem);
int cogen_stmt_break(FILE *fp, stmt_t s, env_t env, label_gen_t lg, stackst_t tmp_mem);
int cogen_stmt_expr(FILE *fp, stmt_t s, env_t env, label_gen_t lg, stackst_t tmp_mem);
int cogen_stmt_compound(FILE *fp, stmt_t s, env_t env, label_gen_t lg, stackst_t tmp_mem);
int cogen_stmt_if(FILE *fp, stmt_t s, env_t env, label_gen_t lg, stackst_t tmp_mem);
int cogen_stmt_while(FILE *fp, stmt_t s, env_t env, label_gen_t lg, stackst_t tmp_mem);
int cogen_expr(FILE *fp, expr_t e, env_t env, stackst_t stack, stackst_t tmp_mem);
int cogen_expr_int_literal(FILE *fp, expr_t e, env_t env, stackst_t stack, stackst_t tmp_mem);
int cogen_expr_id(FILE *fp, expr_t e, env_t env, stackst_t stack, stackst_t tmp_mem);
int cogen_expr_paren(FILE *fp, expr_t e, env_t env, stackst_t stack, stackst_t tmp_mem);
int cogen_expr_app(FILE *fp, expr_t e, env_t env, stackst_t stack, stackst_t tmp_mem);

void cogen_program_header(FILE * fp, char *f);
void cogen_program_trailer(FILE *fp);
void cogen_fun_def_header(FILE *fp, char *f);
void cogen_fun_def_trailer(FILE *fp, char *f);
void cogen_prologue(FILE *fp, fun_def_t d, int frame_sz, int max_reg);
void cogen_epilogue(FILE * fp, fun_def_t d, int frame_sz, int max_reg);

/* セマンティックエラー表示関数 */
void semantic_error(int line);

/**********************************/
/* アセンブリ言語の命令を出力する関数群 */
/**********************************/

/* ジャンプ命令 */
void jl(FILE *fp, int label);
void jle(FILE *fp, int label);
void jg(FILE *fp, int label);
void jge(FILE *fp, int label);
void je(FILE *fp, int label);
void jne(FILE *fp, int label);
void jmp(FILE *fp, int label);

/* ラベルを出力する関数*/
void print_label(FILE *fp, int label);

/* 演算命令 */
void op_const(FILE *fp, op_kind_t op, char *s, int addr);
void op_mem_reg(FILE *fp, op_kind_t op, int addr1, int addr2);
void negl(FILE *fp, int addr);
void addl_mem_reg(FILE *fp, int addr1, int addr2);
void subl_mem_reg(FILE *fp, int addr1, int addr2);
void imull_mem_reg(FILE *fp, int addr1, int addr2);


/* 比較関数 */
void cmp_const(FILE *fp, char *c, int addr);
void cmp_const_int(FILE *fp, int s, int addr);
void cmp_mem_reg(FILE *fp, int addr1, int add2);

/* movl関数 */
void movl_const(FILE *fp, char *c, int addr);
void movl_mem_reg(FILE *fp, int addr1, int addr2);
void movzbl_mem_reg(FILE *fp, int addr);
void opt_div_const(FILE *fp, char *s, int addr);
void opt_div_const_mem(FILE *fp, int addr, char *s);
void opt_div_mem_reg(FILE *fp, int addr1, int addr2);
void opt_rem_const(FILE *fp, char *s, int addr);
void opt_rem_const_mem(FILE *fp, int addr, char *s);
void opt_rem_mem_reg(FILE *fp, int addr1, int addr2);


/* pushl関数 */
void pushl_const(FILE *fp, char *s);
void pushl_mem_reg(FILE *fp, int addr);

/* call関数*/
void call_fun(FILE *fp, char *fun);

/* link math.h problem */
int tmax(int x, int y);
int is_register(int addr);

/* debug */
void find_bug(char *s);

#endif
