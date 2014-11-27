#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "tokenizer.h"

/* トークンの名称 */
const char token_name[28][25] =
{
	"TOK_ID",		    // 変数名
	"TOK_INT",		    // int
	"TOK_INT_LITERAL",	// 整数
	"TOK_BREAK",		// break
	"TOK_CONTINUE",		// continue
	"TOK_IF",		    // if
	"TOK_ELSE",		    // else
	"TOK_RETURN",		// return
	"TOK_WHILE",		// while
	"TOK_LPAREN",		// (
	"TOK_RPAREN",		// )
	"TOK_LBRACE",		// {
	"TOK_RBRACE",		// }
	"TOK_PLUS",		    // +
	"TOK_MINUS",		// -
	"TOK_MUL",		    // *
	"TOK_DIV",		    // /
	"TOK_REM",		    // %
	"TOK_BANG",		    // !
	"TOK_LT",		    // <
	"TOK_GT",		    // >
	"TOK_LE",		    // <=
	"TOK_GE",		    // >=
	"TOK_EQ",		    // ==
	"TOK_NEQ",		    // !=
	"TOK_SEMICOLON",	// ;
	"TOK_ASSIGN",		// =
	"TOK_COMMA"		    // ,
};

extern const char token_name[28][25];

/* tokenizerの初期化 */
tokenizer_t mk_tokenizer(char *filename)
{
	tokenizer_t t = (tokenizer_t) malloc(sizeof(struct tokenizer));
	t->line = 1;
    t->filename = filename;
	t->fp = fopen(filename, "rb");
	if (t->fp == NULL)
		return NULL;
	t->c = fgetc(t->fp);
	next_tok(t);
	return t;
}

/* 現在のトークンを返す関数 */
token cur_tok(tokenizer_t t)
{
	return t->tok;
}

/* 次のトークンを求める関数 */
void next_tok(tokenizer_t t)
{
    t->tok.filename = t->filename;
	while (t->c == ' ' || t->c == '\n' || t->c == '\t') {
		if (t->c == '\n')
			t->line += 1;
		t->c = fgetc(t->fp);
	}
	if (t->c == EOF) {
		t->tok.kind = TOK_EOF;
		return;
	}
	// 整数の場合
	if (isdigit(t->c)) {
		int i = 0;
		char *a = (char *) malloc(11);
		while (isdigit(t->c)) {
			a[i++] = t->c;
			t->c = fgetc(t->fp);
		}
		a[i] = '\0';
		assert(i < 11);
		t->tok.ival = atoi(a);
        t->tok.tok_id = a;
		t->tok.kind = TOK_INT_LITERAL;
		t->tok.line = t->line;
	}
	// '+'の場合
	else if (t->c == '+') {
		t->tok.kind = TOK_PLUS;
		t->tok.line = t->line;
		t->c = fgetc(t->fp);
	}
	// '-'の場合
	else if (t->c == '-') {
		t->tok.kind = TOK_MINUS;
		t->tok.line = t->line;
		t->c = fgetc(t->fp);
	}
	// '*'の場合
	else if (t->c == '*') {
		t->tok.kind = TOK_MUL;
		t->tok.line = t->line;
		t->c = fgetc(t->fp);
	}
	// '/'の場合
	else if (t->c == '/') {
		t->tok.kind = TOK_DIV;
		t->tok.line = t->line;
		t->c = fgetc(t->fp);
	}
	// '('の場合
	else if (t->c == '(') {
		t->tok.kind = TOK_LPAREN;
		t->tok.line = t->line;
		t->c = fgetc(t->fp);
	}
	// ')'の場合
	else if (t->c == ')') {
		t->tok.kind = TOK_RPAREN;
		t->tok.line = t->line;
		t->c = fgetc(t->fp);
	}
	// '{'の場合
	else if (t->c == '{') {
		t->tok.kind = TOK_LBRACE;
		t->tok.line = t->line;
		t->c = fgetc(t->fp);
	}
	// '}'の場合
	else if (t->c == '}') {
		t->tok.kind = TOK_RBRACE;
		t->tok.line = t->line;
		t->c = fgetc(t->fp);
	}
	// '%'の場合
	else if (t->c == '%') {
		t->tok.kind = TOK_REM;
		t->tok.line = t->line;
		t->c = fgetc(t->fp);
	}
	// ';'の場合
	else if (t->c == ';') {
		t->tok.kind = TOK_SEMICOLON;
		t->tok.line = t->line;
		t->c = fgetc(t->fp);
	}
	// ','の場合
	else if (t->c == ',') {
		t->tok.kind = TOK_COMMA;
		t->tok.line = t->line;
		t->c = fgetc(t->fp);
	}
	// '!'で始まる場合
	else if (t->c == '!') {
		t->c = fgetc(t->fp);
		// '!='の場合
		if (t->c == '=') {
			t->tok.kind = TOK_NEQ;
			t->c = fgetc(t->fp);
		}
		// '!'の場合
		else {
			t->tok.kind = TOK_BANG;
		}
		t->tok.line = t->line;
	}
	// '<'で始まる場合
	else if (t->c == '<') {
		t->c = fgetc(t->fp);
		// '<='の場合
		if (t->c == '=') {
			t->tok.kind = TOK_LE;
			t->c = fgetc(t->fp);
		}
		// '<'の場合
		else {
			t->tok.kind = TOK_LT;
		}
		t->tok.line = t->line;
	}
	// '>'で始まる場合
	else if (t->c == '>') {
		t->c = fgetc(t->fp);
		// '>='の場合
		if (t->c == '=') {
			t->tok.kind = TOK_GE;
			t->c = fgetc(t->fp);
		}
		// '>'の場合
		else {
			t->tok.kind = TOK_GT;
		}
		t->tok.line = t->line;
	}
	// '='で始まる場合
	else if (t->c == '=') {
		t->c = fgetc(t->fp);
		// '=='の場合
		if (t->c == '=') {
			t->tok.kind = TOK_EQ;
			t->c = fgetc(t->fp);
		}
		// '='の場合
		else {
			t->tok.kind = TOK_ASSIGN;
		}
		t->tok.line = t->line;
	}
	// 文字列の場合
	else if (is_nondigit(t->c)) {
		char *a = (char *) malloc(10);
		int i = 0, max_size = 10;
		a[i++] = t->c;
		t->c = fgetc(t->fp);
		// 数字・文字である限り、最後まで読み取る
		while (is_nondigit(t->c) || isdigit(t->c)) {
			if (i + 1 == max_size) {
				a = (char *) realloc(a, max_size + 1);
				max_size += 1;
			}
			a[i++] = t->c;
			t->c = fgetc(t->fp);
		}
		a[i++] = '\0';
		// "int"の場合
		if (strcmp(a, "int") == 0) {
			t->tok.kind = TOK_INT;
		}
		// "break"の場合
		else if (strcmp(a, "break") == 0) {
			t->tok.kind = TOK_BREAK;
		}
		// "continue"の場合
		else if (strcmp(a, "continue") == 0) {
			t->tok.kind = TOK_CONTINUE;
		}
		// "if"の場合
		else if (strcmp(a, "if") == 0) {
			t->tok.kind = TOK_IF;
		}
		// "else"の場合
		else if (strcmp(a, "else") == 0) {
			t->tok.kind = TOK_ELSE;
		}
		// "return"の場合
		else if (strcmp(a, "return") == 0) {
			t->tok.kind = TOK_RETURN;
		}
		// "while"の場合
		else if (strcmp(a, "while") == 0) {
			t->tok.kind = TOK_WHILE;
		}
		// 変数名の場合
		else {
			t->tok.kind = TOK_ID;
		}
		t->tok.tok_id = a;
		t->tok.line = t->line;
	}
	// 違法な文字の場合
	else {
		syntax_error(t);
	}
}

/* トークンの情報を表示する関数 */
void print_token(token tok)
{
	if (tok.kind == TOK_INT_LITERAL) {
		fprintf(stdout, "%d:%s (%d)\n", tok.line,
			token_name[tok.kind], tok.ival);
	} else if (tok.kind == TOK_ID) {
		fprintf(stdout, "%d:%s (%s)\n", tok.line,
			token_name[tok.kind], tok.tok_id);
	} else {
		fprintf(stdout, "%d:%s\n", tok.line,
			token_name[tok.kind]);
	}
}

/* 文字あるいは'_'の場合 */
int is_nondigit(char c)
{
	if (c >= 'a' && c <= 'z')
		return 1;
	if (c >= 'A' && c <= 'Z')
		return 1;
	if (c == '_')
		return 1;
	return 0;
}

/* tok_kindのトークンを読み飛ばす */
void eat_it(tokenizer_t t, int tok_kind)
{
    if (cur_tok(t).kind != tok_kind) {
        syntax_error(t);
    }
    next_tok(t);
}

/* エラーを表示する関数 */
void syntax_error(tokenizer_t t)
{
    fprintf(stderr, "syntax error at line %d!\n", cur_tok(t).line);
    exit(1);
}
