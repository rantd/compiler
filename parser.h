#ifndef PARSER_H
#define PARSER_H

#include "syntree.h"
#include "tokenizer.h"

program_t parse_program(tokenizer_t t);
fun_def_t parse_func(tokenizer_t t);

stmt_t parse_stmt(tokenizer_t t);
stmt_t parse_stmt_continue(tokenizer_t);
stmt_t parse_stmt_break(tokenizer_t t);
stmt_t parse_stmt_return(tokenizer_t t);
stmt_t parse_stmt_compound(tokenizer_t t);
stmt_t parse_stmt_if(tokenizer_t t);
stmt_t parse_stmt_while(tokenizer_t t);
stmt_t parse_stmt_expr(tokenizer_t t);

expr_t parse_expr(tokenizer_t t);
expr_t parse_equal(tokenizer_t t);
expr_t parse_relt(tokenizer_t t);
expr_t parse_add(tokenizer_t t);
expr_t parse_mult(tokenizer_t t);
expr_t parse_unary(tokenizer_t t);

#endif
