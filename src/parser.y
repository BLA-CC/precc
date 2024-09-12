%{

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "ast.h"
#include "parser.h"
#include "lexer.h"
#include "str_pool.h"

int yyerror(YYLTYPE *loc, Ast ast, NodeID *root, yyscan_t scanner, const char *msg);

NodeID last_stmt = NO_ID;


# define YYLLOC_DEFAULT(Cur, Rhs, N)        \
do                                          \
    if (N) {                                \
        (Cur).line = YYRHSLOC(Rhs, 1).line; \
        (Cur).col = YYRHSLOC(Rhs, 1).col;   \
    } else {                                \
        (Cur).line = YYRHSLOC(Rhs, 0).line; \
        (Cur).col = YYRHSLOC(Rhs, 0).col;   \
    }                                       \
while (0)

%}

%code requires {
  typedef void* yyscan_t;
}

%output  "src/parser.c"
%defines "include/parser.h"

%define api.pure
%define api.value.type union
%define parse.trace
%define parse.error verbose
%locations
%define api.location.type {Location}
%parse-param { Ast ast }
%parse-param { NodeID *root }
%initial-action {
    @$.line = 1;
    @$.col = 1;
}

%param { yyscan_t scanner }

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
%token <StrID> TOK_IDENT "identifier"
%token <int64_t> TOK_NUM "number"
%token TOK_ILLEGAL_CHAR "illegal character"

%type <NodeID> input
%type <NodeID> seq
%type <NodeID> stmt
%type <NodeID> decl
%type <NodeID> asgn
%type <NodeID> retn
%type <NodeID> expr

%type <Type> main_type

/* Precedence (increasing) and associativity:
   a+b+c is (a+b)+c: left associativity
   a+b*c is a+(b*c): the precedence of "*" is higher than that of "+". */
%left "+"
%left "*"

%%

input: main_type TOK_MAIN "(" ")" "{" seq[body] "}" { 
     *root = ast_mk_main(ast, @2, $1, $body);
     return yynerrs;
     }

main_type
    : TOK_VOID { $$ = Type_VOID; }
    | TOK_BOOL { $$ = Type_BOOL; }
    | TOK_INT  { $$ = Type_INT; }
    ;

seq
    : /* empty */ { $$ = NO_ID; }
    | seq stmt { $$ = $1 == NO_ID ? $2 : $1; }
    ;

stmt: decl | asgn | retn | error ";" { yyerrok; };

decl
    : TOK_BOOL TOK_IDENT ";" { $$ = ast_mk_decl(ast, @2, last_stmt, Type_BOOL, $2); last_stmt = $$; }
    | TOK_INT TOK_IDENT ";"  { $$ = ast_mk_decl(ast, @2, last_stmt, Type_INT, $2); last_stmt = $$; }
    ;

asgn: TOK_IDENT "=" expr ";" { $$ = ast_mk_asgn(ast, @2, last_stmt, $1, $3); last_stmt = $$; };

retn
    : TOK_RETURN ";"      { $$ = ast_mk_ret(ast, @1, last_stmt, NO_ID); last_stmt = $$; }
    | TOK_RETURN expr ";" { $$ = ast_mk_ret(ast, @1, last_stmt, $2); last_stmt = $$; }

expr
    : expr[L] "+" expr[R] { $$ = ast_mk_binop(ast, @2, $L, $R, BinOp_ADD); }
    | expr[L] "*" expr[R] { $$ = ast_mk_binop(ast, @2, $L, $R, BinOp_MUL); }
    | "(" expr[E] ")"     { $$ = $E; }
    | TOK_IDENT           { $$ = ast_mk_var(ast, @1, $1); }
    | TOK_NUM             { $$ = ast_mk_int(ast, @1, $1); }
    | TOK_TRUE            { $$ = ast_mk_bool(ast, @1, true); }
    | TOK_FALSE           { $$ = ast_mk_bool(ast, @1, false); }
    ;

%%

int yyerror(YYLTYPE *loc, Ast ast, NodeID *root, yyscan_t scanner, const char *msg) {
    (void) ast, (void) root, (void) scanner;

    fprintf(stderr, "%d:%d: %s\n", loc->line, loc->col, msg);
    return 1;
}
