%{

#include <stdlib.h>
#include <stdio.h>

#include "ast.h"
#include "parser.h"

%}

%option outfile="src/lexer.c" header-file="include/lexer.h"
%option warn nodefault

%option reentrant noyywrap never-interactive nounistd
%option bison-bridge

WHITESPACE [ \r\n\t]
DIGIT      [0-9]
IDENT      [a-zA-Z][a-zA-Z0-9]*

%%

{WHITESPACE}* { continue; }
{DIGIT}+      { yylval->num = atoll(yytext); return TOK_NUM; }

"return" { return TOK_RETURN; }
"void"   { return TOK_VOID; }
"bool"   { return TOK_BOOL; }
"int"    { return TOK_INT; }

"true"   { return TOK_TRUE; }
"false"  { return TOK_FALSE; }

{IDENT}  { yylval->str = yytext; return TOK_IDENT; }

"*" { return TOK_STAR; }
"+" { return TOK_PLUS; }
"=" { return TOK_EQUAL; }
"(" { return TOK_LPAREN; }
")" { return TOK_RPAREN; }
";" { return TOK_SEMICOLON; }

%%

