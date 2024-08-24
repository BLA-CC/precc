%{

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "ast.h"
#include "parser.h"
#include "lexer.h"

int yyerror(NodePool pool, StmtID *root, yyscan_t scanner, const char *msg);

%}

%code requires {
  typedef void* yyscan_t;
}

%output  "src/parser.c"
%defines "include/parser.h"

%define api.pure
%define api.value.type union
%define parse.trace
%lex-param   { yyscan_t scanner }
// %parse-param { SExpression **expression }
%parse-param { NodePool pool    }
%parse-param { StmtID   *root   }
%parse-param { yyscan_t scanner }

%token TOK_MAIN TOK_RETURN TOK_VOID TOK_BOOL TOK_INT
%token TOK_TRUE TOK_FALSE
%token TOK_PLUS      "+"
%token TOK_STAR      "*"
%token TOK_EQUAL     "="
%token TOK_LPAREN    "("
%token TOK_RPAREN    ")"
%token TOK_LCURLY    "{"
%token TOK_RCURLY    "}"
%token TOK_SEMICOLON ";"
%token <const char*> TOK_IDENT
%token <int64_t> TOK_NUM

%type <id> expr

/* Precedence (increasing) and associativity:
   a+b+c is (a+b)+c: left associativity
   a+b*c is a+(b*c): the precedence of "*" is higher than that of "+". */
%left "+"
%left "*"

%%

input
    : expr { *root = $1; }
    ;

expr
    : expr[L] "+" expr[R] { $$ = pool_binary(pool, $L, $R, BinaryOp_ADD); }
    | expr[L] "*" expr[R] { $$ = pool_binary(pool, $L, $R, BinaryOp_MUL); }
    | "(" expr[E] ")"     { $$ = $E; }
    | TOK_IDENT           { $$ = pool_var(pool, $1); }
    | TOK_NUM             { $$ = pool_int_constant(pool, $1); }
    | TOK_TRUE            { $$ = pool_bool_constant(pool, true); }
    | TOK_FALSE           { $$ = pool_bool_constant(pool, false); }
    ;

%%

int yyerror(NodePool pool, StmtID *root, yyscan_t scanner, const char *msg) {
    (void) pool, (void) root, (void) scanner;

    fprintf(stderr, "error: %s\n", msg);
    return 1;
}
