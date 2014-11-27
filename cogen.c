#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "cogen.h"
#include "list.h"

const char *reg_name[] = {
    "%ebx", "%esi", "%edi", "%esp", "%ebp", "%eax", "%ecx", "%edx"
};

int tmax(int x, int y)
{
    return (x > y) ? x : y;
}

stackst_t mk_stack()
{
    return (stackst_t) mk_list();
}

void stack_push(stackst_t l, int x)
{
    list_add((list_t) l, (void *) x);
}

int stack_sz(stackst_t l)
{
    return list_sz((list_t) l);
}

void stack_swap(stackst_t l)
{
    list_swap((list_t) l);
}

int stack_pop(stackst_t l)
{
    return (int) list_pop((list_t) l);
}

int stack_top(stackst_t l)
{
    return (int) list_top((list_t) l);
}

stackst_t get_stack_register(int n)
{
    stackst_t ans = mk_stack();
    if (n > 0) stack_push(ans, ebx);
    if (n > 1) stack_push(ans, esi);
    if (n > 2) stack_push(ans, edi);
    return ans;
}

label_gen_t mk_label_gen()
{
    label_gen_t lg = (label_gen_t) malloc(sizeof(struct label_gen));
    lg->cnt_compound = 0;
    lg->cnt_return = 0;
    lg->cnt_label = 0;
    lg->list_wh = mk_stack();
    return lg;
}

env_t mk_env()
{
    return (env_t) mk_list();
}

env_t set_env_params(var_decl_list_t params, env_t env)
{
    int i;
    var_decl_t tmp;
    int sz = var_decl_list_sz(params);
    for (i = 0; i < sz; ++i) {
        tmp = var_decl_list_get(params, i);
        tmp->info->save_location = 8 + 4 * i;
        env_add(env, tmp);
    }
    return env;
}
    
void env_add(env_t env, var_decl_t key)
{
    list_add((list_t) env, (void *) key);
}

void *env_pop(env_t env)
{
    int sz = list_sz((list_t) env);
    if (sz == 0) return (void *) NULL;
    return list_pop((list_t) env);
}

var_decl_t env_get(env_t env, int i)
{
    return (var_decl_t) list_get((list_t) env, i);
}

var_decl_t find(env_t env, char *key)
{
    int i;
    var_decl_t tmp;
    int sz = list_sz((list_t) env);
    for (i = sz - 1; i >= 0; --i) {
        tmp = env_get(env, i);
        if (strcmp(key, tmp->v) == 0) return tmp;
    }
    return (var_decl_t) NULL;
}

void semantic_error(int line)
{
    fprintf(stderr, "semantic error at line %d\n", line);
    exit(1);
}

void find_bug(char *s)
{
    fprintf(stdout, "%s\n", s);
}

int cogen_program(FILE *fp, program_t p)
{
    fun_def_t d;
    int i, cnt_label = 0;
    label_gen_t lg;
    cogen_program_header(fp, p->filename);
    fun_def_list_t ds = p->fun_defs;
    int n = fun_def_list_sz(ds);
    for (i = 0; i < n; ++i) {
        d = fun_def_list_get(ds, i);
        lg = mk_label_gen();
        lg->cnt_label = cnt_label;
        cogen_fun_def(fp, d, lg);
        cnt_label = lg->cnt_label + 2;
    }
    cogen_program_trailer(fp);
    return 0;
}

int cogen_fun_def(FILE *fp, fun_def_t d, label_gen_t lg)
{
    int tmp_memory = 0, i;
    int frame_sz = cogen_alloc_storage_fun_def(d, lg, &tmp_memory);
    lg->max_reg = tmp_memory;
    env_t env = mk_env();
    stackst_t tmp_mem = mk_stack();
    for (i = 1; i <= tmp_memory; ++i) {
        stack_push(tmp_mem, -4 * (frame_sz + i + 4));
    }
    frame_sz += tmp_memory;
    env = set_env_params(d->params, env);
    cogen_fun_def_header(fp, d->f);
    cogen_prologue(fp, d, frame_sz, tmp_memory);
    cogen_stmt(fp, d->body, env, lg, tmp_mem);
    if (lg->cnt_return > 1) print_label(fp, lg->cnt_label);
    cogen_epilogue(fp, d, frame_sz, tmp_memory);
    cogen_fun_def_trailer(fp, d->f);
    return 0;
}

int cogen_alloc_storage_fun_def(fun_def_t d, label_gen_t lg, int *tmp_memory)
{
    int frame_sz = 0;
    int save_location = -16;    /* ebp, ebx, esi, edi を利用するため */
    frame_sz = cogen_alloc_storage_stmt(d->body, lg, &save_location, tmp_memory);
    return frame_sz;
}

int cogen_alloc_storage_stmt(stmt_t s, label_gen_t lg, int *save_location, int *tmp_memory)
{
    int i, var_sz, stmt_sz;
    int frame_sz = 0;
    var_decl_t var;
    stmt_t st;
    switch (s->kind) {
        case stmt_kind_empty:
            break;
        case stmt_kind_continue:
        case stmt_kind_break:
            if (stack_sz(lg->list_wh) == 0) semantic_error(s->line);
            s->info->label = stack_top(lg->list_wh);
            break;
        case stmt_kind_return:
            lg->cnt_return += 1;
            frame_sz += cogen_alloc_storage_expr(s->u.e, save_location, 1, tmp_memory);
            break;
        case stmt_kind_expr:
            frame_sz += cogen_alloc_storage_expr(s->u.e, save_location, 1, tmp_memory);
            break;
        case stmt_kind_compound:
            lg->cnt_compound += 1;
            s->info->compound = lg->cnt_compound;
            var_sz = var_decl_list_sz(s->u.c.decls);
            frame_sz += var_sz;
            for (i = 0; i < var_sz; ++i) {
                var = var_decl_list_get(s->u.c.decls, i);
                var->info->save_location = *save_location;
                var->info->compound = s->info->compound;
                *save_location -= 4;
            }
            stmt_sz = stmt_list_sz(s->u.c.body);
            for (i = 0; i < stmt_sz; ++i) {
                st = stmt_list_get(s->u.c.body, i);
                frame_sz += cogen_alloc_storage_stmt(st, lg, save_location, tmp_memory);
            }
            break;
        case stmt_kind_if:
            lg->cnt_label += 1;
            s->info->label = lg->cnt_label;
            lg->cnt_label += 2;
            frame_sz += cogen_alloc_storage_expr(s->u.i.e, save_location, 1, tmp_memory);
            frame_sz += cogen_alloc_storage_stmt(s->u.i.th, lg, save_location, tmp_memory);
            if (s->u.i.el != NULL) {
                frame_sz += cogen_alloc_storage_stmt(s->u.i.el, lg, save_location, tmp_memory);
            }
            break;
        case stmt_kind_while:
            lg->cnt_label += 1;
            s->info->label = lg->cnt_label;
			stack_push(lg->list_wh, lg->cnt_label);
            lg->cnt_label += 3;
            frame_sz += cogen_alloc_storage_expr(s->u.w.e, save_location, 1, tmp_memory);
            frame_sz += cogen_alloc_storage_stmt(s->u.w.body, lg, save_location, tmp_memory);
            stack_pop(lg->list_wh);
            break;
    }
    return frame_sz;
}

int cogen_alloc_storage_expr(expr_t e, int *save_location, int node_tree, int *tmp_memory)
{
    int sz, i;
    int frame_sz = 0;
    expr_list_t args;
    expr_t tmp, e0, e1;
    switch (e->kind) {
        case expr_kind_int_literal:
        case expr_kind_id:
            e->info->node_tree = node_tree;
            *tmp_memory = tmax(*tmp_memory, 1);
            break;
        case expr_kind_paren:
            frame_sz = cogen_alloc_storage_expr(e->u.p, save_location, 1, tmp_memory);
			e->info->node_tree = e->u.p->info->node_tree;
            break;
        case expr_kind_app:
            args = e->u.a.args;
            switch (e->u.a.o) {
                case op_kind_none:
                    break;
                case op_kind_un_plus:
                case op_kind_un_minus:
                case op_kind_logneg:
                    e0 = expr_list_get(args, 0);
                    frame_sz += cogen_alloc_storage_expr(e0, save_location, 1, tmp_memory);
                    e->info->node_tree = e0->info->node_tree;
                    *tmp_memory = tmax(*tmp_memory, e->info->node_tree);
                    break;
                case op_kind_assign:
                case op_kind_eq:
                case op_kind_neq:
                case op_kind_lt:
                case op_kind_gt:
                case op_kind_le:
                case op_kind_ge:
                case op_kind_bin_plus:
                case op_kind_bin_minus:
                case op_kind_mult:
                case op_kind_div:
                case op_kind_rem:
                    e0 = expr_list_get(args, 0);
                    e1 = expr_list_get(args, 1);
                    frame_sz += cogen_alloc_storage_expr(e0, save_location, 1, tmp_memory);
                    frame_sz += cogen_alloc_storage_expr(e1, save_location, 0, tmp_memory);
                    if (e0->info->node_tree == e1->info->node_tree) {
                        e->info->node_tree = e0->info->node_tree + 1;
                    }
                    else {
                        e->info->node_tree = tmax(e0->info->node_tree, e1->info->node_tree);
                    }
                    *tmp_memory = tmax(*tmp_memory, e->info->node_tree);
                    break;
                case op_kind_fun:
                    sz = expr_list_sz(args);
                    e->info->node_tree = node_tree;
                    for (i = 0; i < sz; ++i) {
                        tmp = expr_list_get(args, i);
                        frame_sz += cogen_alloc_storage_expr(tmp, save_location, 1, tmp_memory);
                        e->info->node_tree = tmax(e->info->node_tree, tmp->info->node_tree);
                    }
                    *tmp_memory = tmax(*tmp_memory, e->info->node_tree);
                    break;
            }
    }
    return frame_sz;
}

int cogen_stmt(FILE *fp, stmt_t s, env_t env, label_gen_t lg, stackst_t tmp_mem)
{
    switch (s->kind) {
        case stmt_kind_empty:
            return cogen_stmt_empty(fp, s, env, lg, tmp_mem);
        case stmt_kind_continue:
            return cogen_stmt_continue(fp, s, env, lg, tmp_mem);
        case stmt_kind_break:
            return cogen_stmt_break(fp, s, env, lg, tmp_mem);
        case stmt_kind_return:
            return cogen_stmt_return(fp, s, env, lg, tmp_mem);
        case stmt_kind_expr:
            return cogen_stmt_expr(fp, s, env, lg, tmp_mem);
        case stmt_kind_compound:
            return cogen_stmt_compound(fp, s, env, lg, tmp_mem);
        case stmt_kind_if:
            return cogen_stmt_if(fp, s, env, lg, tmp_mem);
        case stmt_kind_while:
            return cogen_stmt_while(fp, s, env, lg, tmp_mem);
        default:
            assert(0);
    }
    return 0;
}

int cogen_stmt_empty(FILE *fp, stmt_t s, env_t env, label_gen_t lg, stackst_t tmp_mem)
{
    /* nothing to do ^_^ */
    return 0;
}


int cogen_stmt_return(FILE *fp, stmt_t s, env_t env, label_gen_t lg, stackst_t tmp_mem)
{
    expr_t ex;
    var_decl_t var;
    stackst_t stack = get_stack_register(lg->max_reg);
    ex = s->u.e;
    if (ex->kind == expr_kind_int_literal) {
        movl_const(fp, ex->u.s, eax);
    }
    else if (ex->kind == expr_kind_id) {
        var = find(env, ex->u.s);
        if (var == NULL) semantic_error(ex->line);
        movl_mem_reg(fp, var->info->save_location, eax);
    }
    else {
        cogen_expr(fp, s->u.e, env, stack, tmp_mem);
        if (!(s->u.e->kind == expr_kind_app && s->u.e->u.a.o == op_kind_fun)) {
        	movl_mem_reg(fp, s->u.e->info->save_location, eax);
        }
    }
    if (lg->cnt_return > 1) jmp(fp, lg->cnt_label);
    return 0;
}

int cogen_stmt_continue(FILE *fp, stmt_t s, env_t env, label_gen_t lg, stackst_t tmp_mem)
{
    jmp(fp, s->info->label + 1);
    return 0;
}

int cogen_stmt_break(FILE *fp, stmt_t s, env_t env, label_gen_t lg, stackst_t tmp_mem)
{
    jmp(fp, s->info->label + 2);
    return 0;
}

int cogen_stmt_expr(FILE *fp, stmt_t s, env_t env, label_gen_t lg, stackst_t tmp_mem)
{
    stackst_t stack = get_stack_register(lg->max_reg);
    cogen_expr(fp, s->u.e, env, stack, tmp_mem);
    return 0;
}

int cogen_stmt_compound(FILE *fp, stmt_t s, env_t env, label_gen_t lg, stackst_t tmp_mem)
{
    int i;
    stmt_t tmp_stmt;
    var_decl_t tmp_var;
    int var_sz = var_decl_list_sz(s->u.c.decls);
    for (i = 0; i < var_sz; ++i) {
        tmp_var = var_decl_list_get(s->u.c.decls, i);
        env_add(env, tmp_var);
    }
    int stmt_sz = stmt_list_sz(s->u.c.body);
    for (i = 0; i < stmt_sz; ++i) {
        tmp_stmt = stmt_list_get(s->u.c.body, i);
        cogen_stmt(fp, tmp_stmt, env, lg, tmp_mem);
    }
    for (i = 0; i < var_sz; ++i) {
        env_pop(env);
    }
    return 0;
}

int cogen_stmt_if(FILE *fp, stmt_t s, env_t env, label_gen_t lg, stackst_t tmp_mem)
{
    var_decl_t var, vart;
    expr_t e0, e1, e2, e3;
    int val0, val1, val2, val3;
    char *tmp;
    stackst_t stack = get_stack_register(lg->max_reg);
    expr_t e = s->u.i.e;
    if (e->kind == expr_kind_int_literal) {
        int c = atoi(e->u.s);
        if (c == 0) jmp(fp, s->info->label);
    }
    else if (e->kind == expr_kind_id) {
        var = find(env, e->u.s);
        if (var == NULL) semantic_error(e->line);
        cmp_const(fp, "0", var->info->save_location);
        je(fp, s->info->label);
    }
    else if (e->kind == expr_kind_app) {
        switch (e->u.a.o) {
            case op_kind_assign:
            case op_kind_none:
                semantic_error(e->line);
                break;
            case op_kind_fun:
            	cogen_expr(fp, s->u.i.e, env, stack, tmp_mem);
            	cmp_const(fp, "0", eax);
            	je(fp, s->info->label);
            	break;
            case op_kind_eq:		/* a == b */
            	e0 = expr_list_get(e->u.a.args, 0);
            	e1 = expr_list_get(e->u.a.args, 1);
            	if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_int_literal) {
            		val0 = atoi(e0->u.s);
            		val1 = atoi(e1->u.s);
            		if (val0 != val1) jmp(fp, s->info->label);
            		break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_int_literal) {
            		var = find(env, e0->u.s);
					if (var == NULL) semantic_error(e0->line);
					cmp_const(fp, e1->u.s, var->info->save_location);
					jne(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_id) {
            		var = find(env, e1->u.s);
					if (var == NULL) semantic_error(e1->line);
					cmp_const(fp, e0->u.s, var->info->save_location);
					jne(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_id) {
            		var = find(env, e0->u.s);
            		vart = find(env, e1->u.s);
					if (var == NULL || vart == NULL) semantic_error(s->line);
                    movl_mem_reg(fp, vart->info->save_location, eax);
					cmp_mem_reg(fp, eax, var->info->save_location);
					jne(fp, s->info->label);
					break;
            	}
            	if (e1->kind == expr_kind_int_literal) {
            		val0 = atoi(e1->u.s);
            		if (e0->u.a.o == op_kind_rem) {
            			e2 = expr_list_get(e0->u.a.args, 0);
            			e3 = expr_list_get(e0->u.a.args, 1);
            			if (e2->kind == expr_kind_int_literal && e3->kind == expr_kind_int_literal) {
            				val2 = atoi(e2->u.s);
            				val3 = atoi(e3->u.s);
            				if (val3 == 0) semantic_error(e3->line);
            				if (val2 % val3 != val0) jmp(fp, s->info->label);
            				break;
            			}
            			else if (e2->kind == expr_kind_id && e3->kind == expr_kind_int_literal) {
            				var = find(env, e2->u.s);
							if (var == NULL) semantic_error(e2->line);
							val3 = atoi(e3->u.s);
							if (val3 == 0) semantic_error(e3->line);
							movl_const(fp, e3->u.s, ecx);
							opt_rem_mem_reg(fp, ecx, var->info->save_location);
							cmp_const_int(fp, val0, edx);
							jne(fp, s->info->label);
							break;
            			}
            			else if (e2->kind == expr_kind_int_literal && e3->kind == expr_kind_id) {
            				var = find(env, e3->u.s);
							if (var == NULL) semantic_error(e3->line);
							val2 = atoi(e2->u.s);
							if (val2 == 0 && val0 != 0) {
								jmp(fp, s->info->label);
								break;
							}
							opt_rem_const_mem(fp, var->info->save_location, e0->u.s);
							cmp_const_int(fp, val0, edx);
							jne(fp, s->info->label);
							break;
            			}
            			else if (e2->kind == expr_kind_id && e3->kind == expr_kind_id) {
            				var = find(env, e2->u.s);
            				vart = find(env, e3->u.s);
							if (var == NULL || vart == NULL) semantic_error(s->line);
							opt_rem_mem_reg(fp, vart->info->save_location, var->info->save_location);
							cmp_const_int(fp, val0, edx);
							jne(fp, s->info->label);
							break;
            			}
            		}
            	}
            	if (e1->info->node_tree == 0) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_id) {
						var = find(env, e1->u.s);
						if (var == NULL) semantic_error(e1->line);
						cmp_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_int_literal) {
                		cmp_const(fp, e1->u.s, e0->info->save_location);
                	}
					else if (e1->kind == expr_kind_paren) {
						if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                   			movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
						cogen_expr(fp, e1, env, stack, tmp_mem);
						cmp_mem_reg(fp, e1->info->save_location, e0->info->save_location);
					}
                	else {
                    	semantic_error(e1->line);
                	}
            	}
            	else if (e0->info->node_tree < e1->info->node_tree && e0->info->node_tree < stack_sz(stack)) {
                	stack_swap(stack);
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	if (e0->kind == expr_kind_int_literal) {
                		cmp_const(fp, e0->u.s, e1->info->save_location);
                	}
                	else if (e0->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						cmp_mem_reg(fp, var->info->save_location, e1->info->save_location);
                	}
                	else {
                		if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e1->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e0, env, stack, tmp_mem);
                		cmp_mem_reg(fp, e0->info->save_location, reg);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e1->info->node_tree <= e0->info->node_tree && e1->info->node_tree < stack_sz(stack)) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_int_literal) {
                		cmp_const(fp, e1->u.s, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						cmp_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else {
                		if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e1, env, stack, tmp_mem);
                		cmp_mem_reg(fp, e1->info->save_location, reg);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e0->info->node_tree >= stack_sz(stack) && e1->info->node_tree >= stack_sz(stack)) {
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	int mem_addr = stack_pop(tmp_mem);
                	if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    	movl_mem_reg(fp, eax, mem_addr);
                	}
                	else {
                    	movl_mem_reg(fp, stack_top(stack), mem_addr);
                	}
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	cmp_mem_reg(fp, mem_addr, stack_top(stack));
                	stack_push(tmp_mem, mem_addr);
            	}
            	jne(fp, s->info->label);
            	break;
	    	case op_kind_neq:		/* a != b */
            	e0 = expr_list_get(e->u.a.args, 0);
            	e1 = expr_list_get(e->u.a.args, 1);
            	if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_int_literal) {
            		val0 = atoi(e0->u.s);
            		val1 = atoi(e1->u.s);
            		if (val0 == val1) jmp(fp, s->info->label);
            		break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_int_literal) {
            		var = find(env, e0->u.s);
					if (var == NULL) semantic_error(e0->line);
					cmp_const(fp, e1->u.s, var->info->save_location);
					je(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_id) {
            		var = find(env, e1->u.s);
					if (var == NULL) semantic_error(e1->line);
					cmp_const(fp, e0->u.s, var->info->save_location);
					je(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_id) {
            		var = find(env, e0->u.s);
            		vart = find(env, e1->u.s);
					if (var == NULL || vart == NULL) semantic_error(s->line);
                    movl_mem_reg(fp, vart->info->save_location, eax);
					cmp_mem_reg(fp, eax, var->info->save_location);
					je(fp, s->info->label);
					break;
            	}
            	if (e1->info->node_tree == 0) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_id) {
						var = find(env, e1->u.s);
						if (var == NULL) semantic_error(e1->line);
						cmp_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_int_literal) {
                		cmp_const(fp, e1->u.s, e0->info->save_location);
                	}
					else if (e1->kind == expr_kind_paren) {
						if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                   			movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
						cogen_expr(fp, e1, env, stack, tmp_mem);
						cmp_mem_reg(fp, e1->info->save_location, e0->info->save_location);
					}
                	else {
                    	semantic_error(e1->line);
                	}
            	}
            	else if (e0->info->node_tree < e1->info->node_tree && e0->info->node_tree < stack_sz(stack)) {
                	stack_swap(stack);
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	if (e0->kind == expr_kind_int_literal) {
                		cmp_const(fp, e0->u.s, e1->info->save_location);
                	}
                	else if (e0->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						cmp_mem_reg(fp, var->info->save_location, e1->info->save_location);
                	}
                	else {
                		if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e1->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e0, env, stack, tmp_mem);
                		cmp_mem_reg(fp, e0->info->save_location, reg);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e1->info->node_tree <= e0->info->node_tree && e1->info->node_tree < stack_sz(stack)) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_int_literal) {
                		cmp_const(fp, e1->u.s, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						cmp_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else {
                		if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e1, env, stack, tmp_mem);
                		cmp_mem_reg(fp, e1->info->save_location, reg);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e0->info->node_tree >= stack_sz(stack) && e1->info->node_tree >= stack_sz(stack)) {
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	int mem_addr = stack_pop(tmp_mem);
                	if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    	movl_mem_reg(fp, eax, mem_addr);
                	}
                	else {
                    	movl_mem_reg(fp, stack_top(stack), mem_addr);
                	}
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	cmp_mem_reg(fp, mem_addr, stack_top(stack));
                	stack_push(tmp_mem, mem_addr);
            	}
            	je(fp, s->info->label);
            	break;
	    	case op_kind_lt:		/* a < b */
            	e0 = expr_list_get(e->u.a.args, 0);
            	e1 = expr_list_get(e->u.a.args, 1);
            	if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_int_literal) {
            		val0 = atoi(e0->u.s);
            		val1 = atoi(e1->u.s);
            		if (val0 >= val1) jmp(fp, s->info->label);
            		break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_int_literal) {
            		var = find(env, e0->u.s);
					if (var == NULL) semantic_error(e0->line);
					cmp_const(fp, e1->u.s, var->info->save_location);
					jge(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_id) {
            		var = find(env, e1->u.s);
					if (var == NULL) semantic_error(e1->line);
					cmp_const(fp, e0->u.s, var->info->save_location);
					jle(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_id) {
            		var = find(env, e0->u.s);
            		vart = find(env, e1->u.s);
					if (var == NULL || vart == NULL) semantic_error(s->line);
                    movl_mem_reg(fp, vart->info->save_location, eax);
					cmp_mem_reg(fp, eax, var->info->save_location);
					jge(fp, s->info->label);
					break;
            	}
            	if (e1->info->node_tree == 0) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_id) {
						var = find(env, e1->u.s);
						if (var == NULL) semantic_error(e1->line);
						cmp_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_int_literal) {
                		cmp_const(fp, e1->u.s, e0->info->save_location);
                	}
					else if (e1->kind == expr_kind_paren) {
						if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                   			movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
						cogen_expr(fp, e1, env, stack, tmp_mem);
						cmp_mem_reg(fp, e1->info->save_location, e0->info->save_location);
					}
                	else {
                    	semantic_error(e1->line);
                	}
                	jge(fp, s->info->label);
            	}
            	else if (e0->info->node_tree < e1->info->node_tree && e0->info->node_tree < stack_sz(stack)) {
                	stack_swap(stack);
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	if (e0->kind == expr_kind_int_literal) {
                		cmp_const(fp, e0->u.s, e1->info->save_location);
                		jle(fp, s->info->label);
                	}
                	else if (e0->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						cmp_mem_reg(fp, e1->info->save_location, var->info->save_location);
						jge(fp, s->info->label);
                	}
                	else {
                		if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e1->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e0, env, stack, tmp_mem);
                		cmp_mem_reg(fp, reg, e0->info->save_location);
                		jge(fp, s->info->label);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e1->info->node_tree <= e0->info->node_tree && e1->info->node_tree < stack_sz(stack)) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_int_literal) {
                		cmp_const(fp, e1->u.s, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						cmp_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else {
                		if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e1, env, stack, tmp_mem);
                		cmp_mem_reg(fp, e1->info->save_location, reg);
                		stack_push(stack, reg);
            		}
            		jge(fp, s->info->label);
            	}
            	else if (e0->info->node_tree >= stack_sz(stack) && e1->info->node_tree >= stack_sz(stack)) {
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	int mem_addr = stack_pop(tmp_mem);
                	if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    	movl_mem_reg(fp, eax, mem_addr);
                	}
                	else {
                    	movl_mem_reg(fp, stack_top(stack), mem_addr);
                	}
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	cmp_mem_reg(fp, mem_addr, stack_top(stack));
                	jge(fp, s->info->label);
                	stack_push(tmp_mem, mem_addr);
            	}
            	break;
	    	case op_kind_gt:		/* a > b */
            	e0 = expr_list_get(e->u.a.args, 0);
            	e1 = expr_list_get(e->u.a.args, 1);
            	if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_int_literal) {
            		val0 = atoi(e0->u.s);
            		val1 = atoi(e1->u.s);
            		if (val0 <= val1) jmp(fp, s->info->label);
            		break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_int_literal) {
            		var = find(env, e0->u.s);
					if (var == NULL) semantic_error(e0->line);
					cmp_const(fp, e1->u.s, var->info->save_location);
					jle(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_id) {
            		var = find(env, e1->u.s);
					if (var == NULL) semantic_error(e1->line);
					cmp_const(fp, e0->u.s, var->info->save_location);
					jge(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_id) {
            		var = find(env, e0->u.s);
            		vart = find(env, e1->u.s);
					if (var == NULL || vart == NULL) semantic_error(s->line);
                    movl_mem_reg(fp, vart->info->save_location, eax);
					cmp_mem_reg(fp, eax, var->info->save_location);
					jle(fp, s->info->label);
					break;
            	}
            	if (e1->info->node_tree == 0) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_id) {
						var = find(env, e1->u.s);
						if (var == NULL) semantic_error(e1->line);
						cmp_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_int_literal) {
                		cmp_const(fp, e1->u.s, e0->info->save_location);
                	}
					else if (e1->kind == expr_kind_paren) {
						if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                   			movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
						cogen_expr(fp, e1, env, stack, tmp_mem);
						cmp_mem_reg(fp, e1->info->save_location, e0->info->save_location);
					}
                	else {
                    	semantic_error(e1->line);
                	}
                	jle(fp, s->info->label);
            	}
            	else if (e0->info->node_tree < e1->info->node_tree && e0->info->node_tree < stack_sz(stack)) {
                	stack_swap(stack);
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	if (e0->kind == expr_kind_int_literal) {
                		cmp_const(fp, e0->u.s, e1->info->save_location);
                		jge(fp, s->info->label);
                	}
                	else if (e0->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						cmp_mem_reg(fp, e1->info->save_location, var->info->save_location);
						jle(fp, s->info->label);
                	}
                	else {
                		if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e1->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e0, env, stack, tmp_mem);
                		cmp_mem_reg(fp, reg, e0->info->save_location);
                		jle(fp, s->info->label);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e1->info->node_tree <= e0->info->node_tree && e1->info->node_tree < stack_sz(stack)) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_int_literal) {
                		cmp_const(fp, e1->u.s, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						cmp_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else {
                		if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e1, env, stack, tmp_mem);
                		cmp_mem_reg(fp, e1->info->save_location, reg);
                		stack_push(stack, reg);
            		}
            		jle(fp, s->info->label);
            	}
            	else if (e0->info->node_tree >= stack_sz(stack) && e1->info->node_tree >= stack_sz(stack)) {
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	int mem_addr = stack_pop(tmp_mem);
                	if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    	movl_mem_reg(fp, eax, mem_addr);
                	}
                	else {
                    	movl_mem_reg(fp, stack_top(stack), mem_addr);
                	}
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	cmp_mem_reg(fp, mem_addr, stack_top(stack));
                	jle(fp, s->info->label);
                	stack_push(tmp_mem, mem_addr);
            	}
            	break;
	    	case op_kind_le:		/* a <= b */
            	e0 = expr_list_get(e->u.a.args, 0);
            	e1 = expr_list_get(e->u.a.args, 1);
            	if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_int_literal) {
            		val0 = atoi(e0->u.s);
            		val1 = atoi(e1->u.s);
            		if (val0 > val1) jmp(fp, s->info->label);
            		break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_int_literal) {
            		var = find(env, e0->u.s);
					if (var == NULL) semantic_error(e0->line);
					cmp_const(fp, e1->u.s, var->info->save_location);
					jg(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_id) {
            		var = find(env, e1->u.s);
					if (var == NULL) semantic_error(e1->line);
					cmp_const(fp, e0->u.s, var->info->save_location);
					jl(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_id) {
            		var = find(env, e0->u.s);
            		vart = find(env, e1->u.s);
					if (var == NULL || vart == NULL) semantic_error(s->line);
                    movl_mem_reg(fp, vart->info->save_location, eax);
					cmp_mem_reg(fp, eax, var->info->save_location);
					jg(fp, s->info->label);
					break;
            	}
            	if (e1->info->node_tree == 0) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_id) {
						var = find(env, e1->u.s);
						if (var == NULL) semantic_error(e1->line);
						cmp_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_int_literal) {
                		cmp_const(fp, e1->u.s, e0->info->save_location);
                	}
					else if (e1->kind == expr_kind_paren) {
						if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                   			movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
						cogen_expr(fp, e1, env, stack, tmp_mem);
						cmp_mem_reg(fp, e1->info->save_location, e0->info->save_location);
					}
                	else {
                    	semantic_error(e1->line);
                	}
                	jg(fp, s->info->label);
            	}
            	else if (e0->info->node_tree < e1->info->node_tree && e0->info->node_tree < stack_sz(stack)) {
                	stack_swap(stack);
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	if (e0->kind == expr_kind_int_literal) {
                		cmp_const(fp, e0->u.s, e1->info->save_location);
                		jl(fp, s->info->label);
                	}
                	else if (e0->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						cmp_mem_reg(fp, e1->info->save_location, var->info->save_location);
						jg(fp, s->info->label);
                	}
                	else {
                		if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e1->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e0, env, stack, tmp_mem);
                		cmp_mem_reg(fp, reg, e0->info->save_location);
                		jg(fp, s->info->label);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e1->info->node_tree <= e0->info->node_tree && e1->info->node_tree < stack_sz(stack)) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_int_literal) {
                		cmp_const(fp, e1->u.s, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						cmp_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else {
                		if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e1, env, stack, tmp_mem);
                		cmp_mem_reg(fp, e1->info->save_location, reg);
                		stack_push(stack, reg);
            		}
            		jg(fp, s->info->label);
            	}
            	else if (e0->info->node_tree >= stack_sz(stack) && e1->info->node_tree >= stack_sz(stack)) {
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	int mem_addr = stack_pop(tmp_mem);
                	if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    	movl_mem_reg(fp, eax, mem_addr);
                	}
                	else {
                    	movl_mem_reg(fp, stack_top(stack), mem_addr);
                	}
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	cmp_mem_reg(fp, mem_addr, stack_top(stack));
                	jg(fp, s->info->label);
                	stack_push(tmp_mem, mem_addr);
            	}
            	break;
	    	case op_kind_ge:		/* a >= b */
            	e0 = expr_list_get(e->u.a.args, 0);
            	e1 = expr_list_get(e->u.a.args, 1);
            	if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_int_literal) {
            		val0 = atoi(e0->u.s);
            		val1 = atoi(e1->u.s);
            		if (val0 < val1) jmp(fp, s->info->label);
            		break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_int_literal) {
            		var = find(env, e0->u.s);
					if (var == NULL) semantic_error(e0->line);
					cmp_const(fp, e1->u.s, var->info->save_location);
					jl(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_id) {
            		var = find(env, e1->u.s);
					if (var == NULL) semantic_error(e1->line);
					cmp_const(fp, e0->u.s, var->info->save_location);
					jg(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_id) {
            		var = find(env, e0->u.s);
            		vart = find(env, e1->u.s);
					if (var == NULL || vart == NULL) semantic_error(s->line);
                    movl_mem_reg(fp, vart->info->save_location, eax);
					cmp_mem_reg(fp, eax, var->info->save_location);
					jl(fp, s->info->label);
					break;
            	}
            	if (e1->info->node_tree == 0) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_id) {
						var = find(env, e1->u.s);
						if (var == NULL) semantic_error(e1->line);
						cmp_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_int_literal) {
                		cmp_const(fp, e1->u.s, e0->info->save_location);
                	}
					else if (e1->kind == expr_kind_paren) {
						if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                   			movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
						cogen_expr(fp, e1, env, stack, tmp_mem);
						cmp_mem_reg(fp, e1->info->save_location, e0->info->save_location);
					}
                	else {
                    	semantic_error(e1->line);
                	}
                	jl(fp, s->info->label);
            	}
            	else if (e0->info->node_tree < e1->info->node_tree && e0->info->node_tree < stack_sz(stack)) {
                	stack_swap(stack);
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	if (e0->kind == expr_kind_int_literal) {
                		cmp_const(fp, e0->u.s, e1->info->save_location);
                		jg(fp, s->info->label);
                	}
                	else if (e0->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						cmp_mem_reg(fp, e1->info->save_location, var->info->save_location);
						jl(fp, s->info->label);
                	}
                	else {
                		if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e1->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e0, env, stack, tmp_mem);
                		cmp_mem_reg(fp, reg, e0->info->save_location);
                		jl(fp, s->info->label);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e1->info->node_tree <= e0->info->node_tree && e1->info->node_tree < stack_sz(stack)) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_int_literal) {
                		cmp_const(fp, e1->u.s, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						cmp_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else {
                		if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e1, env, stack, tmp_mem);
                		cmp_mem_reg(fp, e1->info->save_location, reg);
                		stack_push(stack, reg);
            		}
            		jl(fp, s->info->label);
            	}
            	else if (e0->info->node_tree >= stack_sz(stack) && e1->info->node_tree >= stack_sz(stack)) {
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	int mem_addr = stack_pop(tmp_mem);
                	if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    	movl_mem_reg(fp, eax, mem_addr);
                	}
                	else {
                    	movl_mem_reg(fp, stack_top(stack), mem_addr);
                	}
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	cmp_mem_reg(fp, mem_addr, stack_top(stack));
                	jl(fp, s->info->label);
                	stack_push(tmp_mem, mem_addr);
            	}
            	break;
	    	case op_kind_bin_plus:	/* a + b */
            	e0 = expr_list_get(e->u.a.args, 0);
            	e1 = expr_list_get(e->u.a.args, 1);
            	if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_int_literal) {
            		val0 = atoi(e0->u.s);
            		val1 = atoi(e1->u.s);
            		if (val0 + val1 == 0) jmp(fp, s->info->label);
            		break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_int_literal) {
            		var = find(env, e0->u.s);
					if (var == NULL) semantic_error(e0->line);
					tmp = (char *) malloc(strlen(e1->u.s) + 1);
					tmp[0] = '-';
					strcpy(tmp + 1, e1->u.s);
					cmp_const(fp, tmp, var->info->save_location);
					je(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_id) {
            		var = find(env, e1->u.s);
					if (var == NULL) semantic_error(e1->line);
					tmp = (char *) malloc(strlen(e0->u.s) + 1);
					tmp[0] = '-';
					strcpy(tmp + 1, e0->u.s);
					cmp_const(fp, tmp, var->info->save_location);
					je(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_id) {
            		var = find(env, e0->u.s);
            		vart = find(env, e1->u.s);
					if (var == NULL || vart == NULL) semantic_error(s->line);
					negl(fp, var->info->save_location);
                    movl_mem_reg(fp, vart->info->save_location, eax);
					cmp_mem_reg(fp, eax, var->info->save_location);
					je(fp, s->info->label);
					break;
            	}
            	if (e1->info->node_tree == 0) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_id) {
						var = find(env, e1->u.s);
						if (var == NULL) semantic_error(e1->line);
						op_mem_reg(fp, op_kind_bin_plus, var->info->save_location, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_int_literal) {
                		op_const(fp, op_kind_bin_plus, e1->u.s, e0->info->save_location);
                	}
					else if (e1->kind == expr_kind_paren) {
						if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                   			movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
						cogen_expr(fp, e1, env, stack, tmp_mem);
						op_mem_reg(fp, op_kind_bin_plus, e1->info->save_location, e0->info->save_location);
					}
                	else {
                    	semantic_error(e1->line);
                	}
            	}
            	else if (e0->info->node_tree < e1->info->node_tree && e0->info->node_tree < stack_sz(stack)) {
                	stack_swap(stack);
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	if (e0->kind == expr_kind_int_literal) {
                		op_const(fp, op_kind_bin_plus, e0->u.s, e1->info->save_location);
                		e0->info->save_location = e1->info->save_location;
                	}
                	else if (e0->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						op_mem_reg(fp, op_kind_bin_plus, e1->info->save_location, var->info->save_location);
						e0->info->save_location = var->info->save_location;
                	}
                	else {
                		if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e1->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e0, env, stack, tmp_mem);
                		op_mem_reg(fp, op_kind_bin_plus, reg, e0->info->save_location);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e1->info->node_tree <= e0->info->node_tree && e1->info->node_tree < stack_sz(stack)) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_int_literal) {
                		op_const(fp, op_kind_bin_plus, e1->u.s, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						op_mem_reg(fp, op_kind_bin_plus, var->info->save_location, e0->info->save_location);
                	}
                	else {
                		if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e1, env, stack, tmp_mem);
                		op_mem_reg(fp, op_kind_bin_plus, e1->info->save_location, reg);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e0->info->node_tree >= stack_sz(stack) && e1->info->node_tree >= stack_sz(stack)) {
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	int mem_addr = stack_pop(tmp_mem);
                	if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    	movl_mem_reg(fp, eax, mem_addr);
                	}
                	else {
                    	movl_mem_reg(fp, stack_top(stack), mem_addr);
                	}
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	op_mem_reg(fp, op_kind_bin_plus, mem_addr, stack_top(stack));
                	e0->info->save_location = stack_top(stack);
                	stack_push(tmp_mem, mem_addr);
            	}
            	cmp_const(fp, "0", e0->info->save_location);
            	je(fp, s->info->label);
            	break;
	    	case op_kind_bin_minus:	/* a - b */
            	e0 = expr_list_get(e->u.a.args, 0);
            	e1 = expr_list_get(e->u.a.args, 1);
            	if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_int_literal) {
            		val0 = atoi(e0->u.s);
            		val1 = atoi(e1->u.s);
            		if (val0 == val1) jmp(fp, s->info->label);
            		break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_int_literal) {
            		var = find(env, e0->u.s);
					if (var == NULL) semantic_error(e0->line);
					cmp_const(fp, e1->u.s, var->info->save_location);
					je(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_id) {
            		var = find(env, e1->u.s);
					if (var == NULL) semantic_error(e1->line);
					cmp_const(fp, e0->u.s, var->info->save_location);
					je(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_id) {
            		var = find(env, e0->u.s);
            		vart = find(env, e1->u.s);
					if (var == NULL || vart == NULL) semantic_error(s->line);
                    movl_mem_reg(fp, vart->info->save_location, eax);
					cmp_mem_reg(fp, eax, var->info->save_location);
					je(fp, s->info->label);
					break;
            	}
            	if (e1->info->node_tree == 0) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_id) {
						var = find(env, e1->u.s);
						if (var == NULL) semantic_error(e1->line);
						op_mem_reg(fp, op_kind_bin_minus, var->info->save_location, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_int_literal) {
                		op_const(fp, op_kind_bin_minus, e1->u.s, e0->info->save_location);
                	}
					else if (e1->kind == expr_kind_paren) {
						if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                   			movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
						cogen_expr(fp, e1, env, stack, tmp_mem);
						op_mem_reg(fp, op_kind_bin_minus, e1->info->save_location, e0->info->save_location);
					}
                	else {
                    	semantic_error(e1->line);
                	}
            	}
            	else if (e0->info->node_tree < e1->info->node_tree && e0->info->node_tree < stack_sz(stack)) {
                	stack_swap(stack);
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	if (e0->kind == expr_kind_int_literal) {
                		op_const(fp, op_kind_bin_minus, e0->u.s, e1->info->save_location);
                		e0->info->save_location = e1->info->save_location;
                	}
                	else if (e0->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						op_mem_reg(fp, op_kind_bin_minus, e1->info->save_location, var->info->save_location);
						e0->info->save_location = var->info->save_location;
                	}
                	else {
                		if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e1->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e0, env, stack, tmp_mem);
                		op_mem_reg(fp, op_kind_bin_minus, reg, e0->info->save_location);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e1->info->node_tree <= e0->info->node_tree && e1->info->node_tree < stack_sz(stack)) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_int_literal) {
                		op_const(fp, op_kind_bin_minus, e1->u.s, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						op_mem_reg(fp, op_kind_bin_minus, var->info->save_location, e0->info->save_location);
                	}
                	else {
                		if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e1, env, stack, tmp_mem);
                		op_mem_reg(fp, op_kind_bin_minus, e1->info->save_location, reg);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e0->info->node_tree >= stack_sz(stack) && e1->info->node_tree >= stack_sz(stack)) {
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	int mem_addr = stack_pop(tmp_mem);
                	if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    	movl_mem_reg(fp, eax, mem_addr);
                	}
                	else {
                    	movl_mem_reg(fp, stack_top(stack), mem_addr);
                	}
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	op_mem_reg(fp, op_kind_bin_minus, mem_addr, stack_top(stack));
                	e0->info->save_location = stack_top(stack);
                	stack_push(tmp_mem, mem_addr);
            	}
            	cmp_const(fp, "0", e0->info->save_location);
            	je(fp, s->info->label);
            	break;
	    	case op_kind_mult:		/* a * b */
            	e0 = expr_list_get(e->u.a.args, 0);
            	e1 = expr_list_get(e->u.a.args, 1);
            	if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_int_literal) {
            		val0 = atoi(e0->u.s);
            		val1 = atoi(e1->u.s);
            		if (val0 == 0 || val1 == 0) jmp(fp, s->info->label);
            		break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_int_literal) {
            		var = find(env, e0->u.s);
					if (var == NULL) semantic_error(e0->line);
					val0 = atoi(e1->u.s);
					if (val0 == 0) {
						jmp(fp, s->info->label);
						break;
					}
					cmp_const(fp, "0", var->info->save_location);
					je(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_id) {
            		var = find(env, e1->u.s);
					if (var == NULL) semantic_error(e1->line);
					val0 = atoi(e0->u.s);
					if (val0 == 0) {
						jmp(fp, s->info->label);
						break;
					}
					cmp_const(fp, "0", var->info->save_location);
					je(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_id) {
            		var = find(env, e0->u.s);
            		vart = find(env, e1->u.s);
					if (var == NULL || vart == NULL) semantic_error(s->line);
					cmp_const(fp, "0", var->info->save_location);
					je(fp, s->info->label);
					cmp_const(fp, "0", vart->info->save_location);
					je(fp, s->info->label);
					break;
            	}
            	if (e1->info->node_tree == 0) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_id) {
						var = find(env, e1->u.s);
						if (var == NULL) semantic_error(e1->line);
						op_mem_reg(fp, op_kind_mult, var->info->save_location, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_int_literal) {
                		op_const(fp, op_kind_mult, e1->u.s, e0->info->save_location);
                	}
					else if (e1->kind == expr_kind_paren) {
						if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                   			movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
						cogen_expr(fp, e1, env, stack, tmp_mem);
						op_mem_reg(fp, op_kind_mult, e1->info->save_location, e0->info->save_location);
					}
                	else {
                    	semantic_error(e1->line);
                	}
            	}
            	else if (e0->info->node_tree < e1->info->node_tree && e0->info->node_tree < stack_sz(stack)) {
                	stack_swap(stack);
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	if (e0->kind == expr_kind_int_literal) {
                		op_const(fp, op_kind_mult, e0->u.s, e1->info->save_location);
                		e0->info->save_location = e1->info->save_location;
                	}
                	else if (e0->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						op_mem_reg(fp, op_kind_mult, e1->info->save_location, var->info->save_location);
						e0->info->save_location = var->info->save_location;
                	}
                	else {
                		if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e1->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e0, env, stack, tmp_mem);
                		op_mem_reg(fp, op_kind_mult, reg, e0->info->save_location);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e1->info->node_tree <= e0->info->node_tree && e1->info->node_tree < stack_sz(stack)) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_int_literal) {
                		op_const(fp, op_kind_mult, e1->u.s, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						op_mem_reg(fp, op_kind_mult, var->info->save_location, e0->info->save_location);
                	}
                	else {
                		if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e1, env, stack, tmp_mem);
                		op_mem_reg(fp, op_kind_mult, e1->info->save_location, reg);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e0->info->node_tree >= stack_sz(stack) && e1->info->node_tree >= stack_sz(stack)) {
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	int mem_addr = stack_pop(tmp_mem);
                	if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    	movl_mem_reg(fp, eax, mem_addr);
                	}
                	else {
                    	movl_mem_reg(fp, stack_top(stack), mem_addr);
                	}
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	op_mem_reg(fp, op_kind_mult, mem_addr, stack_top(stack));
                	e0->info->save_location = stack_top(stack);
                	stack_push(tmp_mem, mem_addr);
            	}
            	cmp_const(fp, "0", e0->info->save_location);
            	je(fp, s->info->label);
            	break;
	    	case op_kind_div:		/* a / b */
            	e0 = expr_list_get(e->u.a.args, 0);
            	e1 = expr_list_get(e->u.a.args, 1);
            	if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_int_literal) {
            		val0 = atoi(e0->u.s);
            		val1 = atoi(e1->u.s);
            		if (val1 == 0) semantic_error(e1->line);
            		if (val0 / val1 == 0) jmp(fp, s->info->label);
            		break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_int_literal) {
            		var = find(env, e0->u.s);
					if (var == NULL) semantic_error(e0->line);
					val0 = atoi(e1->u.s);
					if (val1 == 0) semantic_error(e1->line);
					movl_const(fp, e1->u.s, ecx);
					opt_div_mem_reg(fp, ecx, var->info->save_location);
					cmp_const(fp, "0", eax);
					je(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_id) {
            		var = find(env, e1->u.s);
					if (var == NULL) semantic_error(e1->line);
					val0 = atoi(e0->u.s);
					if (val0 == 0) {
						jmp(fp, s->info->label);
						break;
					}
					opt_div_const_mem(fp, var->info->save_location, e0->u.s);
					cmp_const(fp, "0", eax);
					je(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_id) {
            		var = find(env, e0->u.s);
            		vart = find(env, e1->u.s);
					if (var == NULL || vart == NULL) semantic_error(s->line);
					opt_div_mem_reg(fp, vart->info->save_location, var->info->save_location);
					cmp_const(fp, "0", eax);
					je(fp, s->info->label);
					break;
            	}
            	if (e1->info->node_tree == 0) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_id) {
						var = find(env, e1->u.s);
						if (var == NULL) semantic_error(e1->line);
						opt_div_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_int_literal) {
                		opt_div_const(fp, e1->u.s, e0->info->save_location);
                	}
					else if (e1->kind == expr_kind_paren) {
						if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                   			movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
						cogen_expr(fp, e1, env, stack, tmp_mem);
						opt_div_mem_reg(fp, e1->info->save_location, e0->info->save_location);
					}
                	else {
                    	semantic_error(e1->line);
                	}
            	}
            	else if (e0->info->node_tree < e1->info->node_tree && e0->info->node_tree < stack_sz(stack)) {
                	stack_swap(stack);
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	if (e0->kind == expr_kind_int_literal) {
                		opt_div_const_mem(fp, e1->info->save_location, e0->u.s);
                	}
                	else if (e0->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						opt_div_mem_reg(fp, e1->info->save_location, var->info->save_location);
                	}
                	else {
                		if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e1->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e0, env, stack, tmp_mem);
                		opt_div_mem_reg(fp, reg, e0->info->save_location);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e1->info->node_tree <= e0->info->node_tree && e1->info->node_tree < stack_sz(stack)) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_int_literal) {
                		op_const(fp, op_kind_div, e1->u.s, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						opt_div_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else {
                		if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e1, env, stack, tmp_mem);
                		opt_div_mem_reg(fp, e1->info->save_location, reg);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e0->info->node_tree >= stack_sz(stack) && e1->info->node_tree >= stack_sz(stack)) {
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	int mem_addr = stack_pop(tmp_mem);
                	if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    	movl_mem_reg(fp, eax, mem_addr);
                	}
                	else {
                    	movl_mem_reg(fp, stack_top(stack), mem_addr);
                	}
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	opt_div_mem_reg(fp, mem_addr, stack_top(stack));
                	stack_push(tmp_mem, mem_addr);
            	}
            	cmp_const(fp, "0", eax);
            	je(fp, s->info->label);
            	break;
	    	case op_kind_rem:	    /* a % b */
            	e0 = expr_list_get(e->u.a.args, 0);
            	e1 = expr_list_get(e->u.a.args, 1);
            	if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_int_literal) {
            		val0 = atoi(e0->u.s);
            		val1 = atoi(e1->u.s);
            		if (val1 == 0) semantic_error(e1->line);
            		if (val0 % val1 == 0) jmp(fp, s->info->label);
            		break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_int_literal) {
            		var = find(env, e0->u.s);
					if (var == NULL) semantic_error(e0->line);
					val0 = atoi(e1->u.s);
					if (val1 == 0) semantic_error(e1->line);
					movl_const(fp, e1->u.s, ecx);
					opt_rem_mem_reg(fp, ecx, var->info->save_location);
					cmp_const(fp, "0", edx);
					je(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_id) {
            		var = find(env, e1->u.s);
					if (var == NULL) semantic_error(e1->line);
					val0 = atoi(e0->u.s);
					if (val0 == 0) {
						jmp(fp, s->info->label);
						break;
					}
					opt_rem_const_mem(fp, var->info->save_location, e0->u.s);
					cmp_const(fp, "0", edx);
					je(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_id) {
            		var = find(env, e0->u.s);
            		vart = find(env, e1->u.s);
					if (var == NULL || vart == NULL) semantic_error(s->line);
					opt_rem_mem_reg(fp, vart->info->save_location, var->info->save_location);
					cmp_const(fp, "0", edx);
					je(fp, s->info->label);
					break;
            	}
            	if (e1->info->node_tree == 0) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_id) {
						var = find(env, e1->u.s);
						if (var == NULL) semantic_error(e1->line);
						opt_rem_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_int_literal) {
                		opt_rem_const(fp, e1->u.s, e0->info->save_location);
                	}
					else if (e1->kind == expr_kind_paren) {
						if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                   			movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
						cogen_expr(fp, e1, env, stack, tmp_mem);
						opt_rem_mem_reg(fp, e1->info->save_location, e0->info->save_location);
					}
                	else {
                    	semantic_error(e1->line);
                	}
            	}
            	else if (e0->info->node_tree < e1->info->node_tree && e0->info->node_tree < stack_sz(stack)) {
                	stack_swap(stack);
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	if (e0->kind == expr_kind_int_literal) {
                		opt_rem_const_mem(fp, e1->info->save_location, e0->u.s);
                	}
                	else if (e0->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						opt_rem_mem_reg(fp, e1->info->save_location, var->info->save_location);
                	}
                	else {
                		if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e1->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e0, env, stack, tmp_mem);
                		opt_rem_mem_reg(fp, reg, e0->info->save_location);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e1->info->node_tree <= e0->info->node_tree && e1->info->node_tree < stack_sz(stack)) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_int_literal) {
                		opt_rem_const(fp, e1->u.s, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						opt_rem_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else {
                		if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e1, env, stack, tmp_mem);
                		opt_rem_mem_reg(fp, e1->info->save_location, reg);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e0->info->node_tree >= stack_sz(stack) && e1->info->node_tree >= stack_sz(stack)) {
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	int mem_addr = stack_pop(tmp_mem);
                	if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    	movl_mem_reg(fp, eax, mem_addr);
                	}
                	else {
                    	movl_mem_reg(fp, stack_top(stack), mem_addr);
                	}
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	opt_rem_mem_reg(fp, mem_addr, stack_top(stack));
                	stack_push(tmp_mem, mem_addr);
            	}
            	cmp_const(fp, "0", edx);
            	je(fp, s->info->label);
            	break;
        	case op_kind_un_plus:	/* +a 単項+ */
            case op_kind_un_minus:	/* -a 単項- */
            	e0 = expr_list_get(e->u.a.args, 0);
            	if (e0->kind == expr_kind_int_literal) {
            		int val = atoi(e->u.s);
            		if (val == 0) jmp(fp, s->info->label);
            	}
            	else if (e0->kind == expr_kind_id) {
            		var = find(env, e0->u.s);
					if (var == NULL) semantic_error(e0->line);
					cmp_const(fp, "0", var->info->save_location);
					je(fp, s->info->label);
            	}
            	else {
            		cogen_expr(fp, e0, env, stack, tmp_mem);
            		cmp_const(fp, "0", e0->info->save_location);
            		je(fp, s->info->label);
            	}
            	break;
        	case op_kind_logneg:		/* !a */
            	e0 = expr_list_get(e->u.a.args, 0);
            	if (e0->kind == expr_kind_int_literal) {
            		int val = atoi(e->u.s);
            		if (val != 0) jmp(fp, s->info->label);
            	}
            	else if (e0->kind == expr_kind_id) {
            		var = find(env, e0->u.s);
					if (var == NULL) semantic_error(e0->line);
					cmp_const(fp, "0", var->info->save_location);
					jne(fp, s->info->label);
            	}
            	else {
            		cogen_expr(fp, e0, env, stack, tmp_mem);
            		cmp_const(fp, "0", e0->info->save_location);
            		jne(fp, s->info->label);
            	}
            	break;
    	}
    }
    else {
    	cogen_expr(fp, s->u.i.e, env, stack, tmp_mem);
    	cmp_const(fp, "0", s->u.i.e->info->save_location);
    	je(fp, s->info->label);
    }
    cogen_stmt(fp, s->u.i.th, env, lg, tmp_mem);
    if (s->u.i.el != NULL) jmp(fp, s->info->label + 1);
    print_label(fp, s->info->label);
    if (s->u.i.el != NULL) {
        cogen_stmt(fp, s->u.i.el, env, lg, tmp_mem);
        print_label(fp, s->info->label + 1);
    }
    return 0;
}

int cogen_stmt_while(FILE *fp, stmt_t s, env_t env, label_gen_t lg, stackst_t tmp_mem)
{
	var_decl_t var, vart;
	int val0, val1, val2, val3;
	char *tmp;
	expr_t e0, e1, e2, e3;
    jmp(fp, s->info->label + 1);
    print_label(fp, s->info->label);
    cogen_stmt(fp, s->u.w.body, env, lg, tmp_mem);
    print_label(fp, s->info->label + 1);
    stackst_t stack = get_stack_register(lg->max_reg);
    expr_t e = s->u.w.e;
    if (e->kind == expr_kind_int_literal) {
        int val = atoi(e->u.s);
        if (val != 0) jmp(fp, s->info->label);
    }
    else if (e->kind == expr_kind_id) {
        var = find(env, e->u.s);
        if (var == NULL) semantic_error(e->line);
        cmp_const(fp, "0", var->info->save_location);
        jne(fp, s->info->label);
    }
    else if (e->kind == expr_kind_app) {
        switch (e->u.a.o) {
            case op_kind_assign:
            case op_kind_none:
                semantic_error(e->line);
                break;
            case op_kind_fun:
            	cogen_expr(fp, s->u.i.e, env, stack, tmp_mem);
            	cmp_const(fp, "0", eax);
            	je(fp, s->info->label);
            	break;
            case op_kind_eq:		/* a == b */
            	e0 = expr_list_get(e->u.a.args, 0);
            	e1 = expr_list_get(e->u.a.args, 1);
            	if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_int_literal) {
            		val0 = atoi(e0->u.s);
            		val1 = atoi(e1->u.s);
            		if (val0 == val1) jmp(fp, s->info->label);
            		break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_int_literal) {
            		var = find(env, e0->u.s);
					if (var == NULL) semantic_error(e0->line);
					cmp_const(fp, e1->u.s, var->info->save_location);
					je(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_id) {
            		var = find(env, e1->u.s);
					if (var == NULL) semantic_error(e1->line);
					cmp_const(fp, e0->u.s, var->info->save_location);
					je(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_id) {
            		var = find(env, e0->u.s);
            		vart = find(env, e1->u.s);
					if (var == NULL || vart == NULL) semantic_error(s->line);
                    movl_mem_reg(fp, vart->info->save_location, eax);
					cmp_mem_reg(fp, eax, var->info->save_location);
					je(fp, s->info->label);
					break;
            	}
            	if (e1->kind == expr_kind_int_literal) {
            		val0 = atoi(e1->u.s);
            		if (e0->u.a.o == op_kind_rem) {
            			e2 = expr_list_get(e0->u.a.args, 0);
            			e3 = expr_list_get(e0->u.a.args, 1);
            			if (e2->kind == expr_kind_int_literal && e3->kind == expr_kind_int_literal) {
            				val2 = atoi(e2->u.s);
            				val3 = atoi(e3->u.s);
            				if (val3 == 0) semantic_error(e3->line);
            				if (val2 % val3 == val0) jmp(fp, s->info->label);
            				break;
            			}
            			else if (e2->kind == expr_kind_id && e3->kind == expr_kind_int_literal) {
            				var = find(env, e2->u.s);
							if (var == NULL) semantic_error(e2->line);
							val3 = atoi(e3->u.s);
							if (val3 == 0) semantic_error(e3->line);
							movl_const(fp, e3->u.s, ecx);
							opt_rem_mem_reg(fp, ecx, var->info->save_location);
							cmp_const_int(fp, val0, edx);
							je(fp, s->info->label);
							break;
            			}
            			else if (e2->kind == expr_kind_int_literal && e3->kind == expr_kind_id) {
            				var = find(env, e3->u.s);
							if (var == NULL) semantic_error(e3->line);
							val2 = atoi(e2->u.s);
							if (val2 == 0 && val0 == 0) {
								jmp(fp, s->info->label);
								break;
							}
							opt_rem_const_mem(fp, var->info->save_location, e0->u.s);
							cmp_const_int(fp, val0, edx);
							je(fp, s->info->label);
							break;
            			}
            			else if (e2->kind == expr_kind_id && e3->kind == expr_kind_id) {
            				var = find(env, e2->u.s);
            				vart = find(env, e3->u.s);
							if (var == NULL || vart == NULL) semantic_error(s->line);
							opt_rem_mem_reg(fp, vart->info->save_location, var->info->save_location);
							cmp_const_int(fp, val0, edx);
							je(fp, s->info->label);
							break;
            			}
            		}
            	}
            	if (e1->info->node_tree == 0) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_id) {
						var = find(env, e1->u.s);
						if (var == NULL) semantic_error(e1->line);
						cmp_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_int_literal) {
                		cmp_const(fp, e1->u.s, e0->info->save_location);
                	}
					else if (e1->kind == expr_kind_paren) {
						if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                   			movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
						cogen_expr(fp, e1, env, stack, tmp_mem);
						cmp_mem_reg(fp, e1->info->save_location, e0->info->save_location);
					}
                	else {
                    	semantic_error(e1->line);
                	}
            	}
            	else if (e0->info->node_tree < e1->info->node_tree && e0->info->node_tree < stack_sz(stack)) {
                	stack_swap(stack);
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	if (e0->kind == expr_kind_int_literal) {
                		cmp_const(fp, e0->u.s, e1->info->save_location);
                	}
                	else if (e0->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						cmp_mem_reg(fp, var->info->save_location, e1->info->save_location);
                	}
                	else {
                		if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e1->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e0, env, stack, tmp_mem);
                		cmp_mem_reg(fp, e0->info->save_location, reg);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e1->info->node_tree <= e0->info->node_tree && e1->info->node_tree < stack_sz(stack)) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_int_literal) {
                		cmp_const(fp, e1->u.s, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						cmp_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else {
                		if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e1, env, stack, tmp_mem);
                		cmp_mem_reg(fp, e1->info->save_location, reg);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e0->info->node_tree >= stack_sz(stack) && e1->info->node_tree >= stack_sz(stack)) {
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	int mem_addr = stack_pop(tmp_mem);
                	if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    	movl_mem_reg(fp, eax, mem_addr);
                	}
                	else {
                    	movl_mem_reg(fp, stack_top(stack), mem_addr);
                	}
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	cmp_mem_reg(fp, mem_addr, stack_top(stack));
                	stack_push(tmp_mem, mem_addr);
            	}
            	je(fp, s->info->label);
            	break;
	    	case op_kind_neq:		/* a != b */
            	e0 = expr_list_get(e->u.a.args, 0);
            	e1 = expr_list_get(e->u.a.args, 1);
            	if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_int_literal) {
            		val0 = atoi(e0->u.s);
            		val1 = atoi(e1->u.s);
            		if (val0 != val1) jmp(fp, s->info->label);
            		break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_int_literal) {
            		var = find(env, e0->u.s);
					if (var == NULL) semantic_error(e0->line);
					cmp_const(fp, e1->u.s, var->info->save_location);
					jne(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_id) {
            		var = find(env, e1->u.s);
					if (var == NULL) semantic_error(e1->line);
					cmp_const(fp, e0->u.s, var->info->save_location);
					jne(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_id) {
            		var = find(env, e0->u.s);
            		vart = find(env, e1->u.s);
					if (var == NULL || vart == NULL) semantic_error(s->line);
                    movl_mem_reg(fp, vart->info->save_location, eax);
					cmp_mem_reg(fp, eax, var->info->save_location);
					jne(fp, s->info->label);
					break;
            	}
            	if (e1->info->node_tree == 0) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_id) {
						var = find(env, e1->u.s);
						if (var == NULL) semantic_error(e1->line);
						cmp_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_int_literal) {
                		cmp_const(fp, e1->u.s, e0->info->save_location);
                	}
					else if (e1->kind == expr_kind_paren) {
						if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                   			movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
						cogen_expr(fp, e1, env, stack, tmp_mem);
						cmp_mem_reg(fp, e1->info->save_location, e0->info->save_location);
					}
                	else {
                    	semantic_error(e1->line);
                	}
            	}
            	else if (e0->info->node_tree < e1->info->node_tree && e0->info->node_tree < stack_sz(stack)) {
                	stack_swap(stack);
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	if (e0->kind == expr_kind_int_literal) {
                		cmp_const(fp, e0->u.s, e1->info->save_location);
                	}
                	else if (e0->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						cmp_mem_reg(fp, var->info->save_location, e1->info->save_location);
                	}
                	else {
                		if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e1->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e0, env, stack, tmp_mem);
                		cmp_mem_reg(fp, e0->info->save_location, reg);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e1->info->node_tree <= e0->info->node_tree && e1->info->node_tree < stack_sz(stack)) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_int_literal) {
                		cmp_const(fp, e1->u.s, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						cmp_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else {
                		if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e1, env, stack, tmp_mem);
                		cmp_mem_reg(fp, e1->info->save_location, reg);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e0->info->node_tree >= stack_sz(stack) && e1->info->node_tree >= stack_sz(stack)) {
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	int mem_addr = stack_pop(tmp_mem);
                	if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    	movl_mem_reg(fp, eax, mem_addr);
                	}
                	else {
                    	movl_mem_reg(fp, stack_top(stack), mem_addr);
                	}
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	cmp_mem_reg(fp, mem_addr, stack_top(stack));
                	stack_push(tmp_mem, mem_addr);
            	}
            	jne(fp, s->info->label);
            	break;
	    	case op_kind_lt:		/* a < b */
            	e0 = expr_list_get(e->u.a.args, 0);
            	e1 = expr_list_get(e->u.a.args, 1);
            	if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_int_literal) {
            		val0 = atoi(e0->u.s);
            		val1 = atoi(e1->u.s);
            		if (val0 < val1) jmp(fp, s->info->label);
            		break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_int_literal) {
            		var = find(env, e0->u.s);
					if (var == NULL) semantic_error(e0->line);
					cmp_const(fp, e1->u.s, var->info->save_location);
					jl(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_id) {
            		var = find(env, e1->u.s);
					if (var == NULL) semantic_error(e1->line);
					cmp_const(fp, e0->u.s, var->info->save_location);
					jg(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_id) {
            		var = find(env, e0->u.s);
            		vart = find(env, e1->u.s);
					if (var == NULL || vart == NULL) semantic_error(s->line);
                    movl_mem_reg(fp, vart->info->save_location, eax);
					cmp_mem_reg(fp, eax, var->info->save_location);
					jl(fp, s->info->label);
					break;
            	}
            	if (e1->info->node_tree == 0) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_id) {
						var = find(env, e1->u.s);
						if (var == NULL) semantic_error(e1->line);
						cmp_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_int_literal) {
                		cmp_const(fp, e1->u.s, e0->info->save_location);
                	}
					else if (e1->kind == expr_kind_paren) {
						if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                   			movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
						cogen_expr(fp, e1, env, stack, tmp_mem);
						cmp_mem_reg(fp, e1->info->save_location, e0->info->save_location);
					}
                	else {
                    	semantic_error(e1->line);
                	}
                	jl(fp, s->info->label);
            	}
            	else if (e0->info->node_tree < e1->info->node_tree && e0->info->node_tree < stack_sz(stack)) {
                	stack_swap(stack);
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	if (e0->kind == expr_kind_int_literal) {
                		cmp_const(fp, e0->u.s, e1->info->save_location);
                		jg(fp, s->info->label);
                	}
                	else if (e0->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						cmp_mem_reg(fp, e1->info->save_location, var->info->save_location);
						jl(fp, s->info->label);
                	}
                	else {
                		if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e1->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e0, env, stack, tmp_mem);
                		cmp_mem_reg(fp, reg, e0->info->save_location);
                		jl(fp, s->info->label);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e1->info->node_tree <= e0->info->node_tree && e1->info->node_tree < stack_sz(stack)) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_int_literal) {
                		cmp_const(fp, e1->u.s, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						cmp_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else {
                		if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e1, env, stack, tmp_mem);
                		cmp_mem_reg(fp, e1->info->save_location, reg);
                		stack_push(stack, reg);
            		}
            		jl(fp, s->info->label);
            	}
            	else if (e0->info->node_tree >= stack_sz(stack) && e1->info->node_tree >= stack_sz(stack)) {
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	int mem_addr = stack_pop(tmp_mem);
                	if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    	movl_mem_reg(fp, eax, mem_addr);
                	}
                	else {
                    	movl_mem_reg(fp, stack_top(stack), mem_addr);
                	}
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	cmp_mem_reg(fp, mem_addr, stack_top(stack));
                	jl(fp, s->info->label);
                	stack_push(tmp_mem, mem_addr);
            	}
            	break;
	    	case op_kind_gt:		/* a > b */
            	e0 = expr_list_get(e->u.a.args, 0);
            	e1 = expr_list_get(e->u.a.args, 1);
            	if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_int_literal) {
            		val0 = atoi(e0->u.s);
            		val1 = atoi(e1->u.s);
            		if (val0 > val1) jmp(fp, s->info->label);
            		break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_int_literal) {
            		var = find(env, e0->u.s);
					if (var == NULL) semantic_error(e0->line);
					cmp_const(fp, e1->u.s, var->info->save_location);
					jg(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_id) {
            		var = find(env, e1->u.s);
					if (var == NULL) semantic_error(e1->line);
					cmp_const(fp, e0->u.s, var->info->save_location);
					jl(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_id) {
            		var = find(env, e0->u.s);
            		vart = find(env, e1->u.s);
					if (var == NULL || vart == NULL) semantic_error(s->line);
                    movl_mem_reg(fp, vart->info->save_location, eax);
					cmp_mem_reg(fp, eax, var->info->save_location);
					jg(fp, s->info->label);
					break;
            	}
            	if (e1->info->node_tree == 0) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_id) {
						var = find(env, e1->u.s);
						if (var == NULL) semantic_error(e1->line);
						cmp_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_int_literal) {
                		cmp_const(fp, e1->u.s, e0->info->save_location);
                	}
					else if (e1->kind == expr_kind_paren) {
						if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                   			movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
						cogen_expr(fp, e1, env, stack, tmp_mem);
						cmp_mem_reg(fp, e1->info->save_location, e0->info->save_location);
					}
                	else {
                    	semantic_error(e1->line);
                	}
                	jg(fp, s->info->label);
            	}
            	else if (e0->info->node_tree < e1->info->node_tree && e0->info->node_tree < stack_sz(stack)) {
                	stack_swap(stack);
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	if (e0->kind == expr_kind_int_literal) {
                		cmp_const(fp, e0->u.s, e1->info->save_location);
                		jl(fp, s->info->label);
                	}
                	else if (e0->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						cmp_mem_reg(fp, e1->info->save_location, var->info->save_location);
						jg(fp, s->info->label);
                	}
                	else {
                		if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e1->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e0, env, stack, tmp_mem);
                		cmp_mem_reg(fp, reg, e0->info->save_location);
                		jg(fp, s->info->label);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e1->info->node_tree <= e0->info->node_tree && e1->info->node_tree < stack_sz(stack)) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_int_literal) {
                		cmp_const(fp, e1->u.s, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						cmp_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else {
                		if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e1, env, stack, tmp_mem);
                		cmp_mem_reg(fp, e1->info->save_location, reg);
                		stack_push(stack, reg);
            		}
            		jg(fp, s->info->label);
            	}
            	else if (e0->info->node_tree >= stack_sz(stack) && e1->info->node_tree >= stack_sz(stack)) {
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	int mem_addr = stack_pop(tmp_mem);
                	if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    	movl_mem_reg(fp, eax, mem_addr);
                	}
                	else {
                    	movl_mem_reg(fp, stack_top(stack), mem_addr);
                	}
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	cmp_mem_reg(fp, mem_addr, stack_top(stack));
                	jg(fp, s->info->label);
                	stack_push(tmp_mem, mem_addr);
            	}
            	break;
	    	case op_kind_le:		/* a <= b */
            	e0 = expr_list_get(e->u.a.args, 0);
            	e1 = expr_list_get(e->u.a.args, 1);
            	if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_int_literal) {
            		val0 = atoi(e0->u.s);
            		val1 = atoi(e1->u.s);
            		if (val0 <= val1) jmp(fp, s->info->label);
            		break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_int_literal) {
            		var = find(env, e0->u.s);
					if (var == NULL) semantic_error(e0->line);
					cmp_const(fp, e1->u.s, var->info->save_location);
					jle(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_id) {
            		var = find(env, e1->u.s);
					if (var == NULL) semantic_error(e1->line);
					cmp_const(fp, e0->u.s, var->info->save_location);
					jge(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_id) {
            		var = find(env, e0->u.s);
            		vart = find(env, e1->u.s);
					if (var == NULL || vart == NULL) semantic_error(s->line);
                    movl_mem_reg(fp, vart->info->save_location, eax);
					cmp_mem_reg(fp, eax, var->info->save_location);
					jle(fp, s->info->label);
					break;
            	}
            	if (e1->info->node_tree == 0) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_id) {
						var = find(env, e1->u.s);
						if (var == NULL) semantic_error(e1->line);
						cmp_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_int_literal) {
                		cmp_const(fp, e1->u.s, e0->info->save_location);
                	}
					else if (e1->kind == expr_kind_paren) {
						if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                   			movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
						cogen_expr(fp, e1, env, stack, tmp_mem);
						cmp_mem_reg(fp, e1->info->save_location, e0->info->save_location);
					}
                	else {
                    	semantic_error(e1->line);
                	}
                	jle(fp, s->info->label);
            	}
            	else if (e0->info->node_tree < e1->info->node_tree && e0->info->node_tree < stack_sz(stack)) {
                	stack_swap(stack);
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	if (e0->kind == expr_kind_int_literal) {
                		cmp_const(fp, e0->u.s, e1->info->save_location);
                		jge(fp, s->info->label);
                	}
                	else if (e0->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						cmp_mem_reg(fp, e1->info->save_location, var->info->save_location);
						jle(fp, s->info->label);
                	}
                	else {
                		if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e1->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e0, env, stack, tmp_mem);
                		cmp_mem_reg(fp, reg, e0->info->save_location);
                		jle(fp, s->info->label);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e1->info->node_tree <= e0->info->node_tree && e1->info->node_tree < stack_sz(stack)) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_int_literal) {
                		cmp_const(fp, e1->u.s, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						cmp_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else {
                		if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e1, env, stack, tmp_mem);
                		cmp_mem_reg(fp, e1->info->save_location, reg);
                		stack_push(stack, reg);
            		}
            		jle(fp, s->info->label);
            	}
            	else if (e0->info->node_tree >= stack_sz(stack) && e1->info->node_tree >= stack_sz(stack)) {
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	int mem_addr = stack_pop(tmp_mem);
                	if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    	movl_mem_reg(fp, eax, mem_addr);
                	}
                	else {
                    	movl_mem_reg(fp, stack_top(stack), mem_addr);
                	}
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	cmp_mem_reg(fp, mem_addr, stack_top(stack));
                	jg(fp, s->info->label);
                	stack_push(tmp_mem, mem_addr);
            	}
            	break;
	    	case op_kind_ge:		/* a >= b */
            	e0 = expr_list_get(e->u.a.args, 0);
            	e1 = expr_list_get(e->u.a.args, 1);
            	if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_int_literal) {
            		val0 = atoi(e0->u.s);
            		val1 = atoi(e1->u.s);
            		if (val0 >= val1) jmp(fp, s->info->label);
            		break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_int_literal) {
            		var = find(env, e0->u.s);
					if (var == NULL) semantic_error(e0->line);
					cmp_const(fp, e1->u.s, var->info->save_location);
					jge(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_id) {
            		var = find(env, e1->u.s);
					if (var == NULL) semantic_error(e1->line);
					cmp_const(fp, e0->u.s, var->info->save_location);
					jle(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_id) {
            		var = find(env, e0->u.s);
            		vart = find(env, e1->u.s);
					if (var == NULL || vart == NULL) semantic_error(s->line);
                    movl_mem_reg(fp, vart->info->save_location, eax);
					cmp_mem_reg(fp, eax, var->info->save_location);
					jge(fp, s->info->label);
					break;
            	}
            	if (e1->info->node_tree == 0) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_id) {
						var = find(env, e1->u.s);
						if (var == NULL) semantic_error(e1->line);
						cmp_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_int_literal) {
                		cmp_const(fp, e1->u.s, e0->info->save_location);
                	}
					else if (e1->kind == expr_kind_paren) {
						if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                   			movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
						cogen_expr(fp, e1, env, stack, tmp_mem);
						cmp_mem_reg(fp, e1->info->save_location, e0->info->save_location);
					}
                	else {
                    	semantic_error(e1->line);
                	}
                	jge(fp, s->info->label);
            	}
            	else if (e0->info->node_tree < e1->info->node_tree && e0->info->node_tree < stack_sz(stack)) {
                	stack_swap(stack);
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	if (e0->kind == expr_kind_int_literal) {
                		cmp_const(fp, e0->u.s, e1->info->save_location);
                		jle(fp, s->info->label);
                	}
                	else if (e0->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						cmp_mem_reg(fp, e1->info->save_location, var->info->save_location);
						jge(fp, s->info->label);
                	}
                	else {
                		if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e1->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e0, env, stack, tmp_mem);
                		cmp_mem_reg(fp, reg, e0->info->save_location);
                		jge(fp, s->info->label);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e1->info->node_tree <= e0->info->node_tree && e1->info->node_tree < stack_sz(stack)) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_int_literal) {
                		cmp_const(fp, e1->u.s, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						cmp_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else {
                		if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e1, env, stack, tmp_mem);
                		cmp_mem_reg(fp, e1->info->save_location, reg);
                		stack_push(stack, reg);
            		}
            		jge(fp, s->info->label);
            	}
            	else if (e0->info->node_tree >= stack_sz(stack) && e1->info->node_tree >= stack_sz(stack)) {
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	int mem_addr = stack_pop(tmp_mem);
                	if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    	movl_mem_reg(fp, eax, mem_addr);
                	}
                	else {
                    	movl_mem_reg(fp, stack_top(stack), mem_addr);
                	}
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	cmp_mem_reg(fp, mem_addr, stack_top(stack));
                	jge(fp, s->info->label);
                	stack_push(tmp_mem, mem_addr);
            	}
            	break;
	    	case op_kind_bin_plus:	/* a + b */
            	e0 = expr_list_get(e->u.a.args, 0);
            	e1 = expr_list_get(e->u.a.args, 1);
            	if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_int_literal) {
            		val0 = atoi(e0->u.s);
            		val1 = atoi(e1->u.s);
            		if (val0 + val1 != 0) jmp(fp, s->info->label);
            		break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_int_literal) {
            		var = find(env, e0->u.s);
					if (var == NULL) semantic_error(e0->line);
					tmp = (char *) malloc(strlen(e1->u.s) + 1);
					tmp[0] = '-';
					strcpy(tmp + 1, e1->u.s);
					cmp_const(fp, tmp, var->info->save_location);
					jne(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_id) {
            		var = find(env, e1->u.s);
					if (var == NULL) semantic_error(e1->line);
					tmp = (char *) malloc(strlen(e0->u.s) + 1);
					tmp[0] = '-';
					strcpy(tmp + 1, e0->u.s);
					cmp_const(fp, tmp, var->info->save_location);
					jne(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_id) {
            		var = find(env, e0->u.s);
            		vart = find(env, e1->u.s);
					if (var == NULL || vart == NULL) semantic_error(s->line);
					negl(fp, var->info->save_location);
                    movl_mem_reg(fp, vart->info->save_location, eax);
					cmp_mem_reg(fp, eax, var->info->save_location);
					jne(fp, s->info->label);
					break;
            	}
            	if (e1->info->node_tree == 0) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_id) {
						var = find(env, e1->u.s);
						if (var == NULL) semantic_error(e1->line);
						op_mem_reg(fp, op_kind_bin_plus, var->info->save_location, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_int_literal) {
                		op_const(fp, op_kind_bin_plus, e1->u.s, e0->info->save_location);
                	}
					else if (e1->kind == expr_kind_paren) {
						if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                   			movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
						cogen_expr(fp, e1, env, stack, tmp_mem);
						op_mem_reg(fp, op_kind_bin_plus, e1->info->save_location, e0->info->save_location);
					}
                	else {
                    	semantic_error(e1->line);
                	}
            	}
            	else if (e0->info->node_tree < e1->info->node_tree && e0->info->node_tree < stack_sz(stack)) {
                	stack_swap(stack);
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	if (e0->kind == expr_kind_int_literal) {
                		op_const(fp, op_kind_bin_plus, e0->u.s, e1->info->save_location);
                		e0->info->save_location = e1->info->save_location;
                	}
                	else if (e0->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						op_mem_reg(fp, op_kind_bin_plus, e1->info->save_location, var->info->save_location);
						e0->info->save_location = var->info->save_location;
                	}
                	else {
                		if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e1->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e0, env, stack, tmp_mem);
                		op_mem_reg(fp, op_kind_bin_plus, reg, e0->info->save_location);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e1->info->node_tree <= e0->info->node_tree && e1->info->node_tree < stack_sz(stack)) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_int_literal) {
                		op_const(fp, op_kind_bin_plus, e1->u.s, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						op_mem_reg(fp, op_kind_bin_plus, var->info->save_location, e0->info->save_location);
                	}
                	else {
                		if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e1, env, stack, tmp_mem);
                		op_mem_reg(fp, op_kind_bin_plus, e1->info->save_location, reg);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e0->info->node_tree >= stack_sz(stack) && e1->info->node_tree >= stack_sz(stack)) {
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	int mem_addr = stack_pop(tmp_mem);
                	if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    	movl_mem_reg(fp, eax, mem_addr);
                	}
                	else {
                    	movl_mem_reg(fp, stack_top(stack), mem_addr);
                	}
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	op_mem_reg(fp, op_kind_bin_plus, mem_addr, stack_top(stack));
                	e0->info->save_location = stack_top(stack);
                	stack_push(tmp_mem, mem_addr);
            	}
            	cmp_const(fp, "0", e0->info->save_location);
            	jne(fp, s->info->label);
            	break;
	    	case op_kind_bin_minus:	/* a - b */
            	e0 = expr_list_get(e->u.a.args, 0);
            	e1 = expr_list_get(e->u.a.args, 1);
            	if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_int_literal) {
            		val0 = atoi(e0->u.s);
            		val1 = atoi(e1->u.s);
            		if (val0 != val1) jmp(fp, s->info->label);
            		break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_int_literal) {
            		var = find(env, e0->u.s);
					if (var == NULL) semantic_error(e0->line);
					cmp_const(fp, e1->u.s, var->info->save_location);
					jne(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_id) {
            		var = find(env, e1->u.s);
					if (var == NULL) semantic_error(e1->line);
					cmp_const(fp, e0->u.s, var->info->save_location);
					jne(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_id) {
            		var = find(env, e0->u.s);
            		vart = find(env, e1->u.s);
					if (var == NULL || vart == NULL) semantic_error(s->line);
                    movl_mem_reg(fp, vart->info->save_location, eax);
					cmp_mem_reg(fp, eax, var->info->save_location);
					jne(fp, s->info->label);
					break;
            	}
            	if (e1->info->node_tree == 0) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_id) {
						var = find(env, e1->u.s);
						if (var == NULL) semantic_error(e1->line);
						op_mem_reg(fp, op_kind_bin_minus, var->info->save_location, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_int_literal) {
                		op_const(fp, op_kind_bin_minus, e1->u.s, e0->info->save_location);
                	}
					else if (e1->kind == expr_kind_paren) {
						if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                   			movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
						cogen_expr(fp, e1, env, stack, tmp_mem);
						op_mem_reg(fp, op_kind_bin_minus, e1->info->save_location, e0->info->save_location);
					}
                	else {
                    	semantic_error(e1->line);
                	}
            	}
            	else if (e0->info->node_tree < e1->info->node_tree && e0->info->node_tree < stack_sz(stack)) {
                	stack_swap(stack);
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	if (e0->kind == expr_kind_int_literal) {
                		op_const(fp, op_kind_bin_minus, e0->u.s, e1->info->save_location);
                		e0->info->save_location = e1->info->save_location;
                	}
                	else if (e0->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						op_mem_reg(fp, op_kind_bin_minus, e1->info->save_location, var->info->save_location);
						e0->info->save_location = var->info->save_location;
                	}
                	else {
                		if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e1->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e0, env, stack, tmp_mem);
                		op_mem_reg(fp, op_kind_bin_minus, reg, e0->info->save_location);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e1->info->node_tree <= e0->info->node_tree && e1->info->node_tree < stack_sz(stack)) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_int_literal) {
                		op_const(fp, op_kind_bin_minus, e1->u.s, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						op_mem_reg(fp, op_kind_bin_minus, var->info->save_location, e0->info->save_location);
                	}
                	else {
                		if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e1, env, stack, tmp_mem);
                		op_mem_reg(fp, op_kind_bin_minus, e1->info->save_location, reg);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e0->info->node_tree >= stack_sz(stack) && e1->info->node_tree >= stack_sz(stack)) {
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	int mem_addr = stack_pop(tmp_mem);
                	if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    	movl_mem_reg(fp, eax, mem_addr);
                	}
                	else {
                    	movl_mem_reg(fp, stack_top(stack), mem_addr);
                	}
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	op_mem_reg(fp, op_kind_bin_minus, mem_addr, stack_top(stack));
                	e0->info->save_location = stack_top(stack);
                	stack_push(tmp_mem, mem_addr);
            	}
            	cmp_const(fp, "0", e0->info->save_location);
            	jne(fp, s->info->label);
            	break;
	    	case op_kind_mult:		/* a * b */
            	e0 = expr_list_get(e->u.a.args, 0);
            	e1 = expr_list_get(e->u.a.args, 1);
            	if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_int_literal) {
            		val0 = atoi(e0->u.s);
            		val1 = atoi(e1->u.s);
            		if (val0 != 0 && val1 != 0) jmp(fp, s->info->label);
            		break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_int_literal) {
            		var = find(env, e0->u.s);
					if (var == NULL) semantic_error(e0->line);
					val0 = atoi(e1->u.s);
					if (val0 == 0) {
						jmp(fp, s->info->label + 2);
						break;
					}
					cmp_const(fp, "0", var->info->save_location);
					jne(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_id) {
            		var = find(env, e1->u.s);
					if (var == NULL) semantic_error(e1->line);
					val0 = atoi(e0->u.s);
					if (val0 == 0) {
						jmp(fp, s->info->label + 2);
						break;
					}
					cmp_const(fp, "0", var->info->save_location);
					jne(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_id) {
            		var = find(env, e0->u.s);
            		vart = find(env, e1->u.s);
					if (var == NULL || vart == NULL) semantic_error(s->line);
					cmp_const(fp, "0", var->info->save_location);
					je(fp, s->info->label + 2);
					cmp_const(fp, "0", vart->info->save_location);
					je(fp, s->info->label + 2);
					jmp(fp, s->info->label);
					break;
            	}
            	if (e1->info->node_tree == 0) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_id) {
						var = find(env, e1->u.s);
						if (var == NULL) semantic_error(e1->line);
						op_mem_reg(fp, op_kind_mult, var->info->save_location, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_int_literal) {
                		op_const(fp, op_kind_mult, e1->u.s, e0->info->save_location);
                	}
					else if (e1->kind == expr_kind_paren) {
						if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                   			movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
						cogen_expr(fp, e1, env, stack, tmp_mem);
						op_mem_reg(fp, op_kind_mult, e1->info->save_location, e0->info->save_location);
					}
                	else {
                    	semantic_error(e1->line);
                	}
            	}
            	else if (e0->info->node_tree < e1->info->node_tree && e0->info->node_tree < stack_sz(stack)) {
                	stack_swap(stack);
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	if (e0->kind == expr_kind_int_literal) {
                		op_const(fp, op_kind_mult, e0->u.s, e1->info->save_location);
                		e0->info->save_location = e1->info->save_location;
                	}
                	else if (e0->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						op_mem_reg(fp, op_kind_mult, e1->info->save_location, var->info->save_location);
						e0->info->save_location = var->info->save_location;
                	}
                	else {
                		if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e1->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e0, env, stack, tmp_mem);
                		op_mem_reg(fp, op_kind_mult, reg, e0->info->save_location);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e1->info->node_tree <= e0->info->node_tree && e1->info->node_tree < stack_sz(stack)) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_int_literal) {
                		op_const(fp, op_kind_mult, e1->u.s, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						op_mem_reg(fp, op_kind_mult, var->info->save_location, e0->info->save_location);
                	}
                	else {
                		if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e1, env, stack, tmp_mem);
                		op_mem_reg(fp, op_kind_mult, e1->info->save_location, reg);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e0->info->node_tree >= stack_sz(stack) && e1->info->node_tree >= stack_sz(stack)) {
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	int mem_addr = stack_pop(tmp_mem);
                	if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    	movl_mem_reg(fp, eax, mem_addr);
                	}
                	else {
                    	movl_mem_reg(fp, stack_top(stack), mem_addr);
                	}
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	op_mem_reg(fp, op_kind_mult, mem_addr, stack_top(stack));
                	e0->info->save_location = stack_top(stack);
                	stack_push(tmp_mem, mem_addr);
            	}
            	cmp_const(fp, "0", e0->info->save_location);
            	jne(fp, s->info->label);
            	break;
	    	case op_kind_div:		/* a / b */
            	e0 = expr_list_get(e->u.a.args, 0);
            	e1 = expr_list_get(e->u.a.args, 1);
            	if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_int_literal) {
            		val0 = atoi(e0->u.s);
            		val1 = atoi(e1->u.s);
            		if (val1 == 0) semantic_error(e1->line);
            		if (val0 / val1 != 0) jmp(fp, s->info->label);
            		break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_int_literal) {
            		var = find(env, e0->u.s);
					if (var == NULL) semantic_error(e0->line);
					val0 = atoi(e1->u.s);
					if (val1 == 0) semantic_error(e1->line);
					movl_const(fp, e1->u.s, ecx);
					opt_div_mem_reg(fp, ecx, var->info->save_location);
					cmp_const(fp, "0", eax);
					jne(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_id) {
            		var = find(env, e1->u.s);
					if (var == NULL) semantic_error(e1->line);
					val0 = atoi(e0->u.s);
					if (val0 == 0) break;
					opt_div_const_mem(fp, var->info->save_location, e0->u.s);
					cmp_const(fp, "0", eax);
					jne(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_id) {
            		var = find(env, e0->u.s);
            		vart = find(env, e1->u.s);
					if (var == NULL || vart == NULL) semantic_error(s->line);
					opt_div_mem_reg(fp, vart->info->save_location, var->info->save_location);
					cmp_const(fp, "0", eax);
					jne(fp, s->info->label);
					break;
            	}
            	if (e1->info->node_tree == 0) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_id) {
						var = find(env, e1->u.s);
						if (var == NULL) semantic_error(e1->line);
						opt_div_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_int_literal) {
                		opt_div_const(fp, e1->u.s, e0->info->save_location);
                	}
					else if (e1->kind == expr_kind_paren) {
						if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                   			movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
						cogen_expr(fp, e1, env, stack, tmp_mem);
						opt_div_mem_reg(fp, e1->info->save_location, e0->info->save_location);
					}
                	else {
                    	semantic_error(e1->line);
                	}
            	}
            	else if (e0->info->node_tree < e1->info->node_tree && e0->info->node_tree < stack_sz(stack)) {
                	stack_swap(stack);
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	if (e0->kind == expr_kind_int_literal) {
                		opt_div_const_mem(fp, e1->info->save_location, e0->u.s);
                	}
                	else if (e0->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						opt_div_mem_reg(fp, e1->info->save_location, var->info->save_location);
                	}
                	else {
                		if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e1->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e0, env, stack, tmp_mem);
                		opt_div_mem_reg(fp, reg, e0->info->save_location);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e1->info->node_tree <= e0->info->node_tree && e1->info->node_tree < stack_sz(stack)) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_int_literal) {
                		op_const(fp, op_kind_div, e1->u.s, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						opt_div_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else {
                		if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e1, env, stack, tmp_mem);
                		opt_div_mem_reg(fp, e1->info->save_location, reg);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e0->info->node_tree >= stack_sz(stack) && e1->info->node_tree >= stack_sz(stack)) {
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	int mem_addr = stack_pop(tmp_mem);
                	if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    	movl_mem_reg(fp, eax, mem_addr);
                	}
                	else {
                    	movl_mem_reg(fp, stack_top(stack), mem_addr);
                	}
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	opt_div_mem_reg(fp, mem_addr, stack_top(stack));
                	stack_push(tmp_mem, mem_addr);
            	}
            	cmp_const(fp, "0", eax);
            	jne(fp, s->info->label);
            	break;
	    	case op_kind_rem:	    /* a % b */
            	e0 = expr_list_get(e->u.a.args, 0);
            	e1 = expr_list_get(e->u.a.args, 1);
            	if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_int_literal) {
            		val0 = atoi(e0->u.s);
            		val1 = atoi(e1->u.s);
            		if (val1 == 0) semantic_error(e1->line);
            		if (val0 % val1 != 0) jmp(fp, s->info->label);
            		break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_int_literal) {
            		var = find(env, e0->u.s);
					if (var == NULL) semantic_error(e0->line);
					val0 = atoi(e1->u.s);
					if (val1 == 0) semantic_error(e1->line);
					movl_const(fp, e1->u.s, ecx);
					opt_rem_mem_reg(fp, ecx, var->info->save_location);
					cmp_const(fp, "0", edx);
					jne(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_int_literal && e1->kind == expr_kind_id) {
            		var = find(env, e1->u.s);
					if (var == NULL) semantic_error(e1->line);
					val0 = atoi(e0->u.s);
					if (val0 == 0) break;
					opt_rem_const_mem(fp, var->info->save_location, e0->u.s);
					cmp_const(fp, "0", edx);
					jne(fp, s->info->label);
					break;
            	}
            	else if (e0->kind == expr_kind_id && e1->kind == expr_kind_id) {
            		var = find(env, e0->u.s);
            		vart = find(env, e1->u.s);
					if (var == NULL || vart == NULL) semantic_error(s->line);
					opt_rem_mem_reg(fp, vart->info->save_location, var->info->save_location);
					cmp_const(fp, "0", edx);
					jne(fp, s->info->label);
					break;
            	}
            	if (e1->info->node_tree == 0) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_id) {
						var = find(env, e1->u.s);
						if (var == NULL) semantic_error(e1->line);
						opt_rem_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_int_literal) {
                		opt_rem_const(fp, e1->u.s, e0->info->save_location);
                	}
					else if (e1->kind == expr_kind_paren) {
						if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                   			movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
						cogen_expr(fp, e1, env, stack, tmp_mem);
						opt_rem_mem_reg(fp, e1->info->save_location, e0->info->save_location);
					}
                	else {
                    	semantic_error(e1->line);
                	}
            	}
            	else if (e0->info->node_tree < e1->info->node_tree && e0->info->node_tree < stack_sz(stack)) {
                	stack_swap(stack);
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	if (e0->kind == expr_kind_int_literal) {
                		opt_rem_const_mem(fp, e1->info->save_location, e0->u.s);
                	}
                	else if (e0->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						opt_rem_mem_reg(fp, e1->info->save_location, var->info->save_location);
                	}
                	else {
                		if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e1->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e0, env, stack, tmp_mem);
                		opt_rem_mem_reg(fp, reg, e0->info->save_location);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e1->info->node_tree <= e0->info->node_tree && e1->info->node_tree < stack_sz(stack)) {
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	if (e1->kind == expr_kind_int_literal) {
                		opt_rem_const(fp, e1->u.s, e0->info->save_location);
                	}
                	else if (e1->kind == expr_kind_id) {
                		var = find(env, e0->u.s);
						if (var == NULL) semantic_error(e0->line);
						opt_rem_mem_reg(fp, var->info->save_location, e0->info->save_location);
                	}
                	else {
                		if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                    		movl_mem_reg(fp, eax, stack_top(stack));
                    		e0->info->save_location = stack_top(stack);
                		}
                		int reg = stack_pop(stack);
                		cogen_expr(fp, e1, env, stack, tmp_mem);
                		opt_rem_mem_reg(fp, e1->info->save_location, reg);
                		stack_push(stack, reg);
            		}
            	}
            	else if (e0->info->node_tree >= stack_sz(stack) && e1->info->node_tree >= stack_sz(stack)) {
                	cogen_expr(fp, e1, env, stack, tmp_mem);
                	int mem_addr = stack_pop(tmp_mem);
                	if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    	movl_mem_reg(fp, eax, mem_addr);
                	}
                	else {
                    	movl_mem_reg(fp, stack_top(stack), mem_addr);
                	}
                	cogen_expr(fp, e0, env, stack, tmp_mem);
                	opt_rem_mem_reg(fp, mem_addr, stack_top(stack));
                	stack_push(tmp_mem, mem_addr);
            	}
            	cmp_const(fp, "0", edx);
            	jne(fp, s->info->label);
            	break;
        	case op_kind_un_plus:	/* +a 単項+ */
            case op_kind_un_minus:	/* -a 単項- */
            	e0 = expr_list_get(e->u.a.args, 0);
            	if (e0->kind == expr_kind_int_literal) {
            		int val = atoi(e->u.s);
            		if (val != 0) jmp(fp, s->info->label);
            	}
            	else if (e0->kind == expr_kind_id) {
            		var = find(env, e0->u.s);
					if (var == NULL) semantic_error(e0->line);
					cmp_const(fp, "0", var->info->save_location);
					jne(fp, s->info->label);
            	}
            	else {
            		cogen_expr(fp, e0, env, stack, tmp_mem);
            		cmp_const(fp, "0", e0->info->save_location);
            		jne(fp, s->info->label);
            	}
            	break;
        	case op_kind_logneg:		/* !a */
            	e0 = expr_list_get(e->u.a.args, 0);
            	if (e0->kind == expr_kind_int_literal) {
            		int val = atoi(e->u.s);
            		if (val == 0) jmp(fp, s->info->label);
            	}
            	else if (e0->kind == expr_kind_id) {
            		var = find(env, e0->u.s);
					if (var == NULL) semantic_error(e0->line);
					cmp_const(fp, "0", var->info->save_location);
					je(fp, s->info->label);
            	}
            	else {
            		cogen_expr(fp, e0, env, stack, tmp_mem);
            		cmp_const(fp, "0", e0->info->save_location);
            		je(fp, s->info->label);
            	}
            	break;
    	}
    }
    else {
    	cogen_expr(fp, s->u.w.e, env, stack, tmp_mem);
    	cmp_const(fp, "0", s->u.w.e->info->save_location);
    	jne(fp, s->info->label);
    }
    print_label(fp, s->info->label + 2);
	return 0;
}

int cogen_expr(FILE *fp, expr_t e, env_t env, stackst_t stack, stackst_t tmp_mem)
{
    switch (e->kind) {
        case expr_kind_int_literal:
            return cogen_expr_int_literal(fp, e, env, stack, tmp_mem);
        case expr_kind_id:
            return cogen_expr_id(fp, e, env, stack, tmp_mem);
        case expr_kind_paren:
            return cogen_expr_paren(fp, e, env, stack, tmp_mem);
        case expr_kind_app:
            return cogen_expr_app(fp, e, env, stack, tmp_mem);
        default:
            assert(0);
    }
    return 0;
}

int cogen_expr_int_literal(FILE *fp, expr_t e, env_t env, stackst_t stack, stackst_t tmp_mem)
{
    if (e->info->node_tree == 1) {
        movl_const(fp, e->u.s, stack_top(stack));
        e->info->save_location = stack_top(stack);
    }
    return 0;
}

int cogen_expr_id(FILE *fp, expr_t e, env_t env, stackst_t stack, stackst_t tmp_mem)
{
    var_decl_t var = find(env, e->u.s);
    if (var == NULL) semantic_error(e->line);
    e->info->save_location = var->info->save_location;
    if (e->info->node_tree == 1) {
		movl_mem_reg(fp, var->info->save_location, stack_top(stack));
		e->info->save_location = stack_top(stack);
	}
    return 0;
}

int cogen_expr_paren(FILE *fp, expr_t e, env_t env, stackst_t stack, stackst_t tmp_mem)
{
    cogen_expr(fp, e->u.p, env, stack, tmp_mem);
	e->info->save_location = e->u.p->info->save_location;
    return 0;
}

int cogen_expr_app(FILE *fp, expr_t e, env_t env, stackst_t stack, stackst_t tmp_mem)
{
    int i, sz;
    expr_t ex, e0, e1;
    var_decl_t var, tmp_var;
    switch (e->u.a.o) {
        case op_kind_none:
            break;
        case op_kind_assign:    /* a = b */
            e0 = expr_list_get(e->u.a.args, 0);
            e1 = expr_list_get(e->u.a.args, 1);
            if (e0->kind != expr_kind_id) semantic_error(e0->line);
            var = find(env, e0->u.s);
            if (var == NULL) semantic_error(e0->line);
            if (e1->kind == expr_kind_int_literal) {
                movl_const(fp, e1->u.s, var->info->save_location);
            }
            else {
                if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
					cogen_expr(fp, e1, env, stack, tmp_mem);
                    movl_mem_reg(fp, eax, var->info->save_location);
                }
				else if (e1->kind == expr_kind_id) {
					tmp_var = find(env, e1->u.s);
					if (tmp_var == NULL) semantic_error(e1->line);
					movl_mem_reg(fp, tmp_var->info->save_location, ecx);
					movl_mem_reg(fp, ecx, var->info->save_location);
				}
                else {
					cogen_expr(fp, e1, env, stack, tmp_mem);
                    movl_mem_reg(fp, stack_top(stack), var->info->save_location);
                }
            }
            break;
	    case op_kind_eq:		/* a == b */
	    case op_kind_neq:		/* a != b */
	    case op_kind_lt:		/* a < b */
	    case op_kind_gt:		/* a > b */
	    case op_kind_le:		/* a <= b */
	    case op_kind_ge:		/* a >= b */
	    case op_kind_bin_plus:	/* a + b */
	    case op_kind_bin_minus:	/* a - b */
	    case op_kind_mult:		/* a * b */
	    case op_kind_div:		/* a / b */
	    case op_kind_rem:	    /* a % b */
            e0 = expr_list_get(e->u.a.args, 0);
            e1 = expr_list_get(e->u.a.args, 1);
            if (e1->info->node_tree == 0) {
                cogen_expr(fp, e0, env, stack, tmp_mem);
                if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                    movl_mem_reg(fp, eax, stack_top(stack));
                    e0->info->save_location = stack_top(stack);
                }
                if (e1->kind == expr_kind_id) {
					var = find(env, e1->u.s);
					if (var == NULL) semantic_error(e1->line);
                    op_mem_reg(fp, e->u.a.o, var->info->save_location, stack_top(stack));
                }
                else if (e1->kind == expr_kind_int_literal) {
                    op_const(fp, e->u.a.o, e1->u.s, stack_top(stack));
                }
				else if (e1->kind == expr_kind_paren) {
					cogen_expr(fp, e1, env, stack, tmp_mem);
					op_mem_reg(fp, e->u.a.o, e1->info->save_location, stack_top(stack));
				}
                else {
                    semantic_error(e1->line);
                }
                e->info->save_location = stack_top(stack);
            }
            else if (e0->info->node_tree < e1->info->node_tree && e0->info->node_tree < stack_sz(stack)) {
                stack_swap(stack);
                cogen_expr(fp, e1, env, stack, tmp_mem);
                if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    movl_mem_reg(fp, eax, stack_top(stack));
                    e1->info->save_location = stack_top(stack);
                }
                int reg = stack_pop(stack);
                cogen_expr(fp, e0, env, stack, tmp_mem);
                if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                    movl_mem_reg(fp, eax, stack_top(stack));
                    e0->info->save_location = stack_top(stack);
                }
                op_mem_reg(fp, e->u.a.o, reg, stack_top(stack));
                stack_push(stack, reg);
                stack_swap(stack);
                e->info->save_location = stack_top(stack);
            }
            else if (e1->info->node_tree <= e0->info->node_tree && e1->info->node_tree < stack_sz(stack)) {
                cogen_expr(fp, e0, env, stack, tmp_mem);
                if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                    movl_mem_reg(fp, eax, stack_top(stack));
                    e0->info->save_location = stack_top(stack);
                }
                int reg = stack_pop(stack);
                cogen_expr(fp, e1, env, stack, tmp_mem);
                if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    op_mem_reg(fp, e->u.a.o, eax, reg);
                }
                else {
                    op_mem_reg(fp, e->u.a.o, stack_top(stack), reg);
                }
                stack_push(stack, reg);
                e->info->save_location = reg;
            }
            else if (e0->info->node_tree >= stack_sz(stack) && e1->info->node_tree >= stack_sz(stack)) {
                cogen_expr(fp, e1, env, stack, tmp_mem);
                int mem_addr = stack_pop(tmp_mem);
                if (e1->kind == expr_kind_app && e1->u.a.o == op_kind_fun) {
                    movl_mem_reg(fp, eax, mem_addr);
                }
                else {
                    movl_mem_reg(fp, stack_top(stack), mem_addr);
                }
                cogen_expr(fp, e0, env, stack, tmp_mem);
                if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                    movl_mem_reg(fp, eax, stack_top(stack));
                }
                op_mem_reg(fp, e->u.a.o, mem_addr, stack_top(stack));
                stack_push(tmp_mem, mem_addr);
                e->info->save_location = stack_top(stack);
            }
            break;
        case op_kind_un_plus:	/* +a 単項+ */
            e0 = expr_list_get(e->u.a.args, 0);
            cogen_expr(fp, e0, env, stack, tmp_mem);
            e->info->save_location = e0->info->save_location;
            break;
        case op_kind_un_minus:	/* -a 単項- */
            e0 = expr_list_get(e->u.a.args, 0);
            if (e0->kind == expr_kind_int_literal) {
                char *tmp = (char *) malloc(strlen(e0->u.s) + 1);
                tmp[0] = '-';
                strcpy(tmp + 1, e0->u.s);
                movl_const(fp, tmp, stack_top(stack));
                e->info->save_location = stack_top(stack);
            }
            else {
                cogen_expr(fp, e0, env, stack, tmp_mem);
                if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                    movl_mem_reg(fp, eax, stack_top(stack));
                }
                negl(fp, stack_top(stack));
                e->info->save_location = stack_top(stack);
            }
            break;
        case op_kind_logneg:		/* !a */
            e0 = expr_list_get(e->u.a.args, 0);
            cogen_expr(fp, e0, env, stack, tmp_mem);
            if (e0->kind == expr_kind_app && e0->u.a.o == op_kind_fun) {
                movl_mem_reg(fp, eax, stack_top(stack));
            }
            op_const(fp, op_kind_eq, "0", stack_top(stack));
            e->info->save_location = stack_top(stack);
            break;
        case op_kind_fun:   /* f(x, y, z, ...) */
            sz = expr_list_sz(e->u.a.args);
            for (i = sz - 1; i >= 0; --i) {
                ex = expr_list_get(e->u.a.args, i);
                if (ex->kind == expr_kind_int_literal) {
                    pushl_const(fp, ex->u.s);
                }
                else {
                    cogen_expr(fp, ex, env, stack, tmp_mem);
                    pushl_mem_reg(fp, ex->info->save_location);
                }
            }
            call_fun(fp, e->u.a.f);
            if (sz > 0) fprintf(fp, "\taddl\t$%d, %%esp\n", 4 * sz);
            e->info->save_location = eax;
            break;
    }
    return 0;
}

void cogen_program_header(FILE * fp, char *f)
{
    fprintf(fp, "\t.file\t\"%s\"\n", f);
    fprintf(fp, "\t.text\n");
}

void cogen_program_trailer(FILE *fp)
{
    fprintf(fp, "\t.ident \"GCC: (Ubuntu 4.3.3-5ubuntu4) 4.3.3\"\n");
    fprintf(fp, "\t.section\t.note.GNU-stack,\"\",@progbits\n");
}

void cogen_fun_def_header(FILE *fp, char *f)
{
    fprintf(fp, "\t.globl\t%s\n", f);
    fprintf(fp, "\t.type\t%s, @function\n", f);
    fprintf(fp, "%s:\n", f);
}

void cogen_fun_def_trailer(FILE *fp, char *f)
{	
    fprintf(fp, "\t.size %s, .-%s\n", f, f);
}	
	
void cogen_prologue(FILE *fp, fun_def_t d, int frame_sz, int max_reg)
{
    fprintf(fp, "\t.cfi_startproc\n");
    fprintf(fp, "\tpushl\t%%ebp\n");
    fprintf(fp, "\t.cfi_def_cfa_offset 8\n");
    fprintf(fp, "\t.cfi_offset 5, -8\n");
    fprintf(fp, "\tmovl\t%%esp, %%ebp\n");
    fprintf(fp, "\t.cfi_def_cfa_register 5\n");
    if (max_reg > 0) fprintf(fp, "\tpushl\t%%ebx\n");
    if (max_reg > 1) fprintf(fp, "\tpushl\t%%esi\n");
    if (max_reg > 2) fprintf(fp, "\tpushl\t%%edi\n");
    if(frame_sz > 0){
        fprintf(fp, "\tsubl\t$%d, %%esp\n", 4 * frame_sz);
    }
}

void cogen_epilogue(FILE * fp, fun_def_t d, int frame_sz, int max_reg)
{
    if(frame_sz > 0){
        fprintf(fp, "\taddl\t$%d, %%esp\n", 4 * frame_sz);
    }
    if (max_reg > 2) fprintf(fp, "\tpopl\t%%edi\n");
    if (max_reg > 1) fprintf(fp, "\tpopl\t%%esi\n");
    if (max_reg > 0) fprintf(fp, "\tpopl\t%%ebx\n");
    fprintf(fp, "\tleave\n");
    fprintf(fp, "\t.cfi_restore 5\n");
    fprintf(fp, "\t.cfi_def_cfa 4, 4\n");
    fprintf(fp, "\tret\n");
    fprintf(fp, "\t.cfi_endproc\n");
}

void opt_div_const(FILE *fp, char *s, int addr)
{
	movl_mem_reg(fp, addr, eax);
	fprintf(fp, "\tmovl\t$%s, %s\n", s, reg_name[ecx]);
    fprintf(fp, "\tcltd\n");
    fprintf(fp, "\tidivl\t%s\n", reg_name[ecx]);
}

void opt_div_mem_reg(FILE *fp, int addr1, int addr2)
{
	movl_mem_reg(fp, addr2, eax);
    fprintf(fp, "\tcltd\n");
    if (is_register(addr1)) fprintf(fp, "\tidivl\t%s\n", reg_name[addr1]);
    else fprintf(fp, "\tidivl\t%d(%%ebp)\n", addr1);
}

void opt_rem_const(FILE *fp, char *s, int addr)
{
	movl_mem_reg(fp, addr, eax);
	fprintf(fp, "\tmovl\t$%s, %s\n", s, reg_name[ecx]);
    fprintf(fp, "\tcltd\n");
    fprintf(fp, "\tidivl\t%s\n", reg_name[ecx]);
}

void opt_rem_mem_reg(FILE *fp, int addr1, int addr2)
{
	movl_mem_reg(fp, addr2, eax);
    fprintf(fp, "\tcltd\n");
    if (is_register(addr1)) fprintf(fp, "\tidivl\t%s\n", reg_name[addr1]);
    else fprintf(fp, "\tidivl\t%d(%%ebp)\n", addr1);
}


void opt_rem_const_mem(FILE *fp, int addr, char *s)
{
	movl_const(fp, s, eax);
    fprintf(fp, "\tcltd\n");
    if (is_register(addr)) fprintf(fp, "\tidivl\t%s\n", reg_name[addr]);
    else fprintf(fp, "\tidivl\t%d(%%ebp)\n", addr);
}


void opt_div_const_mem(FILE *fp, int addr, char *s)
{
	movl_const(fp, s, eax);
    fprintf(fp, "\tcltd\n");
    if (is_register(addr)) fprintf(fp, "\tidivl\t%s\n", reg_name[addr]);
    else fprintf(fp, "\tidivl\t%d(%%ebp)\n", addr);
}

void op_mem_reg(FILE *fp, op_kind_t op, int addr1, int addr2)
{
    switch (op) {
        case op_kind_assign:
            movl_mem_reg(fp, addr1, addr2);
            break;
        case op_kind_eq:
            cmp_mem_reg(fp, addr1, addr2);
            fprintf(fp, "\tsete\t%%al\n");
            movzbl_mem_reg(fp, addr2);
            break;
        case op_kind_neq:
            cmp_mem_reg(fp, addr1, addr2);
            fprintf(fp, "\tsetne\t%%al\n");
            movzbl_mem_reg(fp, addr2);
            break;
        case op_kind_lt:
            cmp_mem_reg(fp, addr1, addr2);
            fprintf(fp, "\tsetl\t%%al\n");
            movzbl_mem_reg(fp, addr2);
            break;
        case op_kind_gt:
            cmp_mem_reg(fp, addr1, addr2);
            fprintf(fp, "\tsetg\t%%al\n");
            movzbl_mem_reg(fp, addr2);
            break;
        case op_kind_le:
            cmp_mem_reg(fp, addr1, addr2);
            fprintf(fp, "\tsetle\t%%al\n");
            movzbl_mem_reg(fp, addr2);
            break;
        case op_kind_ge:
            cmp_mem_reg(fp, addr1, addr2);
            fprintf(fp, "\tsetge\t%%al\n");
            movzbl_mem_reg(fp, addr2);
            break;
        case op_kind_bin_plus:
            addl_mem_reg(fp, addr1, addr2);
            break;
        case op_kind_bin_minus:
            subl_mem_reg(fp, addr1, addr2);
            break;
        case op_kind_mult:
            imull_mem_reg(fp, addr1, addr2);
            break;
        case op_kind_div:
            movl_mem_reg(fp, addr2, eax);
            fprintf(fp, "\tcltd\n");
            if (is_register(addr1)) fprintf(fp, "\tidivl\t%s\n", reg_name[addr1]);
            else fprintf(fp, "\tidivl\t%d(%%ebp)\n", addr1);
            movl_mem_reg(fp, eax, addr2);
            break;
        case op_kind_rem:
            movl_mem_reg(fp, addr2, eax);
            fprintf(fp, "\tcltd\n");
            if (is_register(addr1)) fprintf(fp, "\tidivl\t%s\n", reg_name[addr1]);
            else fprintf(fp, "\tidivl\t%d(%%ebp)\n", addr1);
            movl_mem_reg(fp, edx, addr2);
            break;
    }
}

void op_const(FILE *fp, op_kind_t op, char *s, int addr)
{
	switch (op) {
        case op_kind_assign:
            movl_const(fp, s, addr);
            break;
        case op_kind_eq:
            cmp_const(fp, s, addr);
            fprintf(fp, "\tsete\t%%al\n");
            movzbl_mem_reg(fp, addr);
            break;
        case op_kind_neq:
            cmp_const(fp, s, addr);
            fprintf(fp, "\tsetne\t%%al\n");
            movzbl_mem_reg(fp, addr);
            break;
        case op_kind_lt:
            cmp_const(fp, s, addr);
            fprintf(fp, "\tsetl\t%%al\n");
            movzbl_mem_reg(fp, addr);
            break;
        case op_kind_gt:
            cmp_const(fp, s, addr);
            fprintf(fp, "\tsetg\t%%al\n");
            movzbl_mem_reg(fp, addr);
            break;
        case op_kind_le:
            cmp_const(fp, s, addr);
            fprintf(fp, "\tsetle\t%%al\n");
            movzbl_mem_reg(fp, addr);
            break;
        case op_kind_ge:
            cmp_const(fp, s, addr);
            fprintf(fp, "\tsetge\t%%al\n");
            movzbl_mem_reg(fp, addr);
            break;
        case op_kind_bin_plus:
            fprintf(fp, "\taddl\t$%s, %s\n", s, reg_name[addr]);
            break;
        case op_kind_bin_minus:
            fprintf(fp, "\tsubl\t$%s, %s\n", s, reg_name[addr]);
            break;
        case op_kind_mult:
            fprintf(fp, "\timull\t$%s, %s\n", s, reg_name[addr]);
            break;
        case op_kind_div:
            movl_mem_reg(fp, addr, eax);
			fprintf(fp, "\tmovl\t$%s, %s\n", s, reg_name[ecx]);
            fprintf(fp, "\tcltd\n");
            fprintf(fp, "\tidivl\t%s\n", reg_name[ecx]);
            movl_mem_reg(fp, eax, addr);
            break;
        case op_kind_rem:
            movl_mem_reg(fp, addr, eax);
            fprintf(fp, "\tmovl\t$%s, %s\n", s, reg_name[ecx]);
            fprintf(fp, "\tcltd\n");
            fprintf(fp, "\tidivl\t%s\n", reg_name[ecx]);
            movl_mem_reg(fp, edx, addr);
            break;
    }
}

void movl_const(FILE *fp, char *s, int addr)
{
	if (is_register(addr)) fprintf(fp, "\tmovl\t$%s, %s\n", s, reg_name[addr]);
    else fprintf(fp, "\tmovl\t$%s, %d(%%ebp)\n", s, addr);
}

void movl_mem_reg(FILE *fp, int addr1, int addr2)
{
	if (is_register(addr1) && is_register(addr2)) fprintf(fp, "\tmovl\t%s, %s\n", reg_name[addr1], reg_name[addr2]);
    else if (is_register(addr1) && !is_register(addr2)) fprintf(fp, "\tmovl\t%s, %d(%%ebp)\n", reg_name[addr1], addr2);
    else if (!is_register(addr1) && is_register(addr2)) fprintf(fp, "\tmovl\t%d(%%ebp), %s\n", addr1, reg_name[addr2]);
    else assert(0);
}

void cmp_const(FILE *fp, char *s, int addr)
{
	if (is_register(addr)) fprintf(fp, "\tcmpl\t$%s, %s\n", s, reg_name[addr]);
    else fprintf(fp, "\tcmpl\t$%s, %d(%%ebp)\n", s, addr);
}

void cmp_const_int(FILE *fp, int s, int addr)
{
	if (is_register(addr)) fprintf(fp, "\tcmpl\t$%d, %s\n", s, reg_name[addr]);
    else fprintf(fp, "\tcmpl\t$%d, %d(%%ebp)\n", s, addr);
}

void cmp_mem_reg(FILE *fp, int addr1, int addr2)
{
    if (is_register(addr1) && is_register(addr2)) fprintf(fp, "\tcmpl\t%s, %s\n", reg_name[addr1], reg_name[addr2]);
    else if (is_register(addr1) && !is_register(addr2)) fprintf(fp, "\tcmpl\t%s, %d(%%ebp)\n", reg_name[addr1], addr2);
	else if (!is_register(addr1) && is_register(addr2)) fprintf(fp, "\tcmpl\t%d(%%ebp), %s\n", addr1, reg_name[addr2]);
    else fprintf(fp, "\tcmpl\t%d(%%ebp), %d(%%ebp)\n", addr1, addr2);
}

void pushl_mem_reg(FILE *fp, int addr)
{
	if (is_register(addr)) fprintf(fp, "\tpushl\t%s\n", reg_name[addr]);
    else fprintf(fp, "\tpushl\t%d(%%ebp)\n", addr);
}

void pushl_const(FILE *fp, char *s)
{
	fprintf(fp, "\tpushl\t$%s\n", s);
}

void call_fun(FILE *fp, char *fun)
{
	fprintf(fp, "\tcall\t%s\n", fun);
}

void jl(FILE *fp, int label)
{
    fprintf(fp, "\tjl\t.L%d\n", label);
}

void jle(FILE *fp, int label)
{
    fprintf(fp, "\tjle\t.L%d\n", label);
}
void jg(FILE *fp, int label)
{
    fprintf(fp, "\tjg\t.L%d\n", label);
}

void jge(FILE *fp, int label)
{
    fprintf(fp, "\tjge\t.L%d\n", label);
}

void je(FILE *fp, int label)
{
    fprintf(fp, "\tje\t.L%d\n", label);
}

void jne(FILE *fp, int label)
{
    fprintf(fp, "\tjne\t.L%d\n", label);
}

void jmp(FILE *fp, int label)
{
    fprintf(fp, "\tjmp\t.L%d\n", label);
}

void movzbl_mem_reg(FILE *fp, int addr)
{
    if (is_register(addr)) fprintf(fp, "\tmovzbl\t%%al, %s\n", reg_name[addr]);
    else fprintf(fp, "\tmovzbl\t%%al, %d(%%ebp)\n", addr);
}

void addl_mem_reg(FILE *fp, int addr1, int addr2)
{
    assert(addr2 >= 0);
    if (is_register(addr1)) fprintf(fp, "\taddl\t%s, %s\n", reg_name[addr1], reg_name[addr2]);
    else fprintf(fp, "\taddl\t%d(%%ebp), %s\n", addr1, reg_name[addr2]);
}

void subl_mem_reg(FILE *fp, int addr1, int addr2)
{
    assert(addr2 >= 0);
    if (is_register(addr1)) fprintf(fp, "\tsubl\t%s, %s\n", reg_name[addr1], reg_name[addr2]);
    else fprintf(fp, "\tsubl\t%d(%%ebp), %s\n", addr1, reg_name[addr2]);
}

void imull_mem_reg(FILE *fp, int addr1, int addr2)
{
    assert(addr2 >= 0);
    if (is_register(addr1)) fprintf(fp, "\timull\t%s, %s\n", reg_name[addr1], reg_name[addr2]);
    else fprintf(fp, "\timull\t%d(%%ebp), %s\n", addr1, reg_name[addr2]);
}

void negl(FILE *fp, int addr)
{
    assert(addr >= 0);
    fprintf(fp, "\tnegl\t%s\n", reg_name[addr]);
}

int is_register(int addr)
{
    if (addr >= 0 && addr < 8) return 1;
    return 0;
}

void print_label(FILE *fp, int label)
{
    fprintf(fp, ".L%d:\n", label);
}
