#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/* トークンの種類の定義 */
typedef enum
{
	TOK_ID,
	TOK_INT,
	TOK_INT_LITERAL,
	TOK_BREAK,
	TOK_CONTINUE,
	TOK_IF,
	TOK_ELSE,
	TOK_RETURN,
	TOK_WHILE,
	TOK_LPAREN,
	TOK_RPAREN,
	TOK_LBRACE,
	TOK_RBRACE,
	TOK_PLUS,
	TOK_MINUS,
	TOK_MUL,
	TOK_DIV,
	TOK_REM,
	TOK_BANG,
	TOK_LT,
	TOK_GT,
	TOK_LE,
	TOK_GE,
	TOK_EQ,
	TOK_NEQ,
	TOK_SEMICOLON,
	TOK_ASSIGN,
	TOK_COMMA,
	TOK_EOF
} token_kind_t;

/* トークンのデータ構造 */
typedef struct token
{
	token_kind_t kind;	// トークンの種類
	int ival;		    // 整数の場合、その値を記憶
	char *tok_id;		// トークンの文字列を記憶
	char *filename;     // ファイル名
    int line;		    // トークンが何行目にあるかを記憶
} token;

/* tokenizerのデータ構造 */
typedef struct tokenizer
{
	token tok;		// 現在のトークン
	char c;			// 先に読み取った文字
	int line;		// 現在の行
    char *filename; // ファイル名
	FILE *fp;		// ファイルへの参照
} *tokenizer_t;

/* ファイルからtokenizerを構築する関数 */
tokenizer_t mk_tokenizer(char *filename);

/* 現在のトークンを返す関数 */
token cur_tok(tokenizer_t t);

/* 次のトークンを求める関数 */
void next_tok(tokenizer_t t);

/* トークンを表示する関数 */
void print_token(token tok);

/* 文字あるいは'_'であるかをチェックする関数 */
int is_nondigit(char c);

/* tok_kindのトークンを読み飛ばす */
void eat_it(tokenizer_t t, int tok_kind);

/* エラーを表示する関数 */
void syntax_error(tokenizer_t t);

#endif
