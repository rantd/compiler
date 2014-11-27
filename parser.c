#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

program_t parse_program(tokenizer_t t)
{
	char *filename = cur_tok(t).filename;
	fun_def_list_t l = mk_fun_def_list();
	while (cur_tok(t).kind != TOK_EOF) {
		fun_def_t x = parse_func(t);
		fun_def_list_add(l, x);
	}
	return mk_program(filename, l);
}

fun_def_t parse_func(tokenizer_t t)
{
	char *filename = cur_tok(t).filename;
	int line = cur_tok(t).line;
	eat_it(t, TOK_INT);
	char *f = cur_tok(t).tok_id;
	eat_it(t, TOK_ID);
	eat_it(t, TOK_LPAREN);
	var_decl_list_t params = mk_var_decl_list();
	while (cur_tok(t).kind != TOK_RPAREN) {
		eat_it(t, TOK_INT);
		var_decl_t x = mk_var_decl(cur_tok(t).filename, cur_tok(t).line, cur_tok(t).tok_id);
		var_decl_list_add(params, x);
		eat_it(t, TOK_ID);
		if (cur_tok(t).kind == TOK_COMMA) {
			eat_it(t, TOK_COMMA);
		} else if (cur_tok(t).kind != TOK_RPAREN) {
			syntax_error(t);
		}
	}
	eat_it(t, TOK_RPAREN);
	stmt_t body = parse_stmt_compound(t);
	return mk_fun_def(filename, line, f, params, body);
}

stmt_t parse_stmt(tokenizer_t t)
{
	switch (cur_tok(t).kind) {
    case TOK_SEMICOLON:
        eat_it(t, TOK_SEMICOLON);
		return mk_stmt_empty(cur_tok(t).filename, cur_tok(t).line);
	case TOK_CONTINUE:
		return parse_stmt_continue(t);
	case TOK_BREAK:
		return parse_stmt_break(t);
	case TOK_RETURN:
		return parse_stmt_return(t);
	case TOK_LBRACE:
		return parse_stmt_compound(t);
	case TOK_IF:
		return parse_stmt_if(t);
	case TOK_WHILE:
		return parse_stmt_while(t);
	default:
		return parse_stmt_expr(t);
	}
}

stmt_t parse_stmt_continue(tokenizer_t t)
{
	char *filename = cur_tok(t).filename;
	int line = cur_tok(t).line;
	eat_it(t, TOK_CONTINUE);
	eat_it(t, TOK_SEMICOLON);
	return mk_stmt_continue(filename, line);
}

stmt_t parse_stmt_break(tokenizer_t t)
{
	char *filename = cur_tok(t).filename;
	int line = cur_tok(t).line;
	eat_it(t, TOK_BREAK);
	eat_it(t, TOK_SEMICOLON);
	return mk_stmt_break(filename, line);
}

stmt_t parse_stmt_return(tokenizer_t t)
{
	char *filename = cur_tok(t).filename;
	int line = cur_tok(t).line;
	eat_it(t, TOK_RETURN);
	expr_t e = parse_expr(t);
	eat_it(t, TOK_SEMICOLON);
	return mk_stmt_return(filename, line, e);
}

stmt_t parse_stmt_compound(tokenizer_t t)
{
	char *filename = cur_tok(t).filename;
	int line = cur_tok(t).line;
	eat_it(t, TOK_LBRACE);
	var_decl_list_t decls = mk_var_decl_list();
	while (cur_tok(t).kind == TOK_INT) {
		eat_it(t, TOK_INT);
		var_decl_t var = mk_var_decl(cur_tok(t).filename, cur_tok(t).line, cur_tok(t).tok_id);
		var_decl_list_add(decls, var);
		eat_it(t, TOK_ID);
		eat_it(t, TOK_SEMICOLON);
	}
	stmt_list_t body = mk_stmt_list();
	while (cur_tok(t).kind != TOK_RBRACE) {
		stmt_t x = parse_stmt(t);
		stmt_list_add(body, x);
	}
	eat_it(t, TOK_RBRACE);
	return mk_stmt_compound(filename, line, decls, body);
}

stmt_t parse_stmt_if(tokenizer_t t)
{
	char *filename = cur_tok(t).filename;
	int line = cur_tok(t).line;
	eat_it(t, TOK_IF);
	eat_it(t, TOK_LPAREN);
	expr_t e = parse_expr(t);
	eat_it(t, TOK_RPAREN);
	stmt_t th = parse_stmt(t);
	stmt_t el = NULL;
	if (cur_tok(t).kind == TOK_ELSE) {
		eat_it(t, TOK_ELSE);
		el = parse_stmt(t);
	}
	return mk_stmt_if(filename, line, e, th, el);
}

stmt_t parse_stmt_while(tokenizer_t t)
{
	char *filename = cur_tok(t).filename;
	int line = cur_tok(t).line;
	eat_it(t, TOK_WHILE);
	eat_it(t, TOK_LPAREN);
	expr_t e = parse_expr(t);
	eat_it(t, TOK_RPAREN);
	stmt_t body = parse_stmt(t);
	return mk_stmt_while(filename, line, e, body);
}

stmt_t parse_stmt_expr(tokenizer_t t)
{
	char *filename = cur_tok(t).filename;
	int line = cur_tok(t).line;
	expr_t e = parse_expr(t);
	eat_it(t, TOK_SEMICOLON);
	return mk_stmt_expr(filename, line, e);
}


expr_t parse_expr(tokenizer_t t)
{
	char *filename = cur_tok(t).filename;
	int line = cur_tok(t).line;
	expr_t e = parse_equal(t);
	if (cur_tok(t).kind == TOK_ASSIGN) {
		eat_it(t, TOK_ASSIGN);
		expr_t ee = parse_expr(t);
		e = mk_expr_bin_op(filename, line, op_kind_assign, e, ee);
	}
    else return e;
}

expr_t parse_unary(tokenizer_t t)
{
	char *filename = cur_tok(t).filename;
	int line = cur_tok(t).line;
	expr_t e;
	char *f;
	switch (cur_tok(t).kind) {
	case TOK_INT_LITERAL:
		eat_it(t, TOK_INT_LITERAL);
		return mk_expr_int_literal(filename, line, cur_tok(t).tok_id);
	case TOK_ID:
		f = cur_tok(t).tok_id;
		eat_it(t, TOK_ID);
		if (cur_tok(t).kind == TOK_LPAREN) {
			eat_it(t, TOK_LPAREN);
			expr_list_t list = mk_expr_list();
			while (cur_tok(t).kind != TOK_RPAREN) {
				e = parse_expr(t);
				expr_list_add(list, e);
				if (cur_tok(t).kind == TOK_COMMA) {
					eat_it(t, TOK_COMMA);
				}
                else if (cur_tok(t).kind != TOK_RPAREN) {
					syntax_error(t);
				}
			}
			eat_it(t, TOK_RPAREN);
			return mk_expr_call(filename, line, f, list);
		}
        else {
			return mk_expr_id(filename, line, f);
		}
	case TOK_LPAREN:
		eat_it(t, TOK_LPAREN);
		e = parse_expr(t);
		eat_it(t, TOK_RPAREN);
		return mk_expr_paren(filename, line, e);
	case TOK_PLUS:
		eat_it(t, TOK_PLUS);
		e = parse_unary(t);
		return mk_expr_un_op(filename, line, op_kind_un_plus, e);
	case TOK_MINUS:
		eat_it(t, TOK_MINUS);
		e = parse_unary(t);
		return mk_expr_un_op(filename, line, op_kind_un_minus, e);
	case TOK_BANG:
		eat_it(t, TOK_BANG);
		e = parse_unary(t);
		return mk_expr_un_op(filename, line, op_kind_logneg, e);
	default:
		syntax_error(t);
	}
}

expr_t parse_mult(tokenizer_t t)
{
	char *filename = cur_tok(t).filename;
	int line = cur_tok(t).line;
	expr_t e = parse_unary(t);
	while (1) {
		int kind = cur_tok(t).kind;
		if (kind == TOK_MUL) {
			eat_it(t, TOK_MUL);
			expr_t ee = parse_unary(t);
			e = mk_expr_bin_op(filename, line, op_kind_mult, e, ee);
		} else if (kind == TOK_DIV) {
			eat_it(t, TOK_DIV);
			expr_t ee = parse_unary(t);
			e = mk_expr_bin_op(filename, line, op_kind_div, e, ee);
		} else if (kind == TOK_REM) {
			eat_it(t, TOK_REM);
			expr_t ee = parse_unary(t);
			e = mk_expr_bin_op(filename, line, op_kind_rem, e, ee);
		} else break;
	}
	return e;
}

expr_t parse_add(tokenizer_t t)
{
	char *filename = cur_tok(t).filename;
	int line = cur_tok(t).line;
	expr_t e = parse_mult(t);
	while (1) {
		int kind = cur_tok(t).kind;
		if (kind == TOK_PLUS) {
			eat_it(t, TOK_PLUS);
			expr_t ee = parse_mult(t);
			e = mk_expr_bin_op(filename, line, op_kind_bin_plus, e, ee);
		} else if (kind == TOK_MINUS) {
			eat_it(t, TOK_MINUS);
			expr_t ee = parse_mult(t);
			e = mk_expr_bin_op(filename, line, op_kind_bin_minus, e, ee);
		} else break;
	}
	return e;
}

expr_t parse_relt(tokenizer_t t)
{
	char *filename = cur_tok(t).filename;
	int line = cur_tok(t).line;
	expr_t e = parse_add(t);
	while (1) {
		int kind = cur_tok(t).kind;
		if (kind == TOK_LT) {
			eat_it(t, TOK_LT);
			expr_t ee = parse_add(t);
			e = mk_expr_bin_op(filename, line, op_kind_lt, e, ee);
		} else if (kind == TOK_GT) {
			eat_it(t, TOK_GT);
			expr_t ee = parse_add(t);
			e = mk_expr_bin_op(filename, line, op_kind_gt, e, ee);
		} else if (kind == TOK_LE) {
			eat_it(t, TOK_LE);
			expr_t ee = parse_add(t);
			e = mk_expr_bin_op(filename, line, op_kind_le, e, ee);
		} else if (kind == TOK_GE) {
			eat_it(t, TOK_GE);
			expr_t ee = parse_add(t);
			e = mk_expr_bin_op(filename, line, op_kind_ge, e, ee);
		} else
			break;
	}
	return e;
}

expr_t parse_equal(tokenizer_t t)
{
	char *filename = cur_tok(t).filename;
	int line = cur_tok(t).line;
	expr_t e = parse_relt(t);
	while (1) {
		int kind = cur_tok(t).kind;
		if (kind == TOK_EQ) {
			eat_it(t, TOK_EQ);
			expr_t ee = parse_relt(t);
			e = mk_expr_bin_op(filename, line, op_kind_eq, e, ee);
		} else if (kind == TOK_NEQ) {
			eat_it(t, TOK_NEQ);
			expr_t ee = parse_relt(t);
			e = mk_expr_bin_op(filename, line, op_kind_neq, e, ee);
		} else break;
	}
	return e;
}
