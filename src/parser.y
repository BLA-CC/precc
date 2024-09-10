%{

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "ast.h"
#include "parser.h"
#include "lexer.h"
#include "str_pool.h"

int yyerror(AST ast, StmtID *root, yyscan_t scanner, const char *msg);

NodeID last_stmt = NO_ID;

%}

%code requires {
  typedef void* yyscan_t;
}

%output  "src/parser.c"
%defines "include/parser.h"

%define api.pure
%define api.value.type union
%define parse.trace
%parse-param { Ast ast    }
%parse-param { NodeID   *root   }

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
%token <StrID> TOK_IDENT
%token <int64_t> TOK_NUM

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
     *root = ast_mk_main(ast, $1, $body);
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

stmt: decl | asgn | retn;

decl
    : TOK_BOOL TOK_IDENT ";" { $$ = ast_mk_decl(ast, last_stmt, Type_BOOL, $2); last_stmt = $$; }
    | TOK_INT TOK_IDENT ";"  { $$ = ast_mk_decl(ast, last_stmt, Type_INT, $2); last_stmt = $$; }
    ;

asgn: TOK_IDENT "=" expr ";" { $$ = ast_mk_asgn(ast, last_stmt, $1, $3); last_stmt = $$; };

retn
    : TOK_RETURN ";"      { $$ = ast_mk_ret(ast, last_stmt, NO_ID); last_stmt = $$; }
    | TOK_RETURN expr ";" { $$ = ast_mk_ret(ast, last_stmt, $2); last_stmt = $$; }

expr
    : expr[L] "+" expr[R] { $$ = ast_mk_binop(ast, $L, $R, BinOp_ADD); }
    | expr[L] "*" expr[R] { $$ = ast_mk_binop(ast, $L, $R, BinOp_MUL); }
    | "(" expr[E] ")"     { $$ = $E; }
    | TOK_IDENT           { $$ = ast_mk_var(ast, $1); }
    | TOK_NUM             { $$ = ast_mk_int(ast, $1); }
    | TOK_TRUE            { $$ = ast_mk_bool(ast, true); }
    | TOK_FALSE           { $$ = ast_mk_bool(ast, false); }
    ;

%%

int yyerror(Ast ast, NodeID *root, yyscan_t scanner, const char *msg) {
    (void) ast, (void) root, (void) scanner;

    fprintf(stderr, "error: %s\n", msg);
    return 1;
}
