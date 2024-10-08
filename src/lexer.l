%{

#include <stdlib.h>
#include <stdio.h>

#include "ast.h"
#include "util.h"
#include "parser.h"
#include "str_pool.h"

#define YY_EXTRA_TYPE StrPool *
%}

%option bison-locations
%option outfile="src/lexer.c" header-file="include/lexer.h"
%option warn nodefault noyywrap nounput noinput

%option reentrant noyywrap never-interactive nounistd
%option bison-bridge

WS      [ \t]
NL      \r\n|\n\r|\n
DIGIT   [0-9]
IDENT   [a-zA-Z][a-zA-Z0-9]*

%%
%{
    yylloc->col += yyleng;
%}

{WS}+    { yylloc->col += yyleng; }
{NL}+    { yylloc->col = 1; yylloc->line += yyleng; }

"main"   { return TOK_MAIN; }
"return" { return TOK_RETURN; }
"void"   { return TOK_VOID; }
"bool"   { return TOK_BOOL; }
"int"    { return TOK_INT; }

"true"   { return TOK_TRUE; }
"false"  { return TOK_FALSE; }

"*" { return TOK_STAR; }
"+" { return TOK_PLUS; }
"=" { return TOK_EQUAL; }
"(" { return TOK_LPAREN; }
")" { return TOK_RPAREN; }
"{" { return TOK_LCURLY; }
"}" { return TOK_RCURLY; }
";" { return TOK_SEMICOLON; }

{IDENT}  { yylval->TOK_IDENT = str_pool_put(*yyextra, yytext); return TOK_IDENT; }
{DIGIT}+ { yylval->TOK_NUM = atoll(yytext); return TOK_NUM; }

. { return TOK_ILLEGAL_CHAR; }

%%

