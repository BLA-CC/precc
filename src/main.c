#include <stdio.h>

#include "ast.h"
#include "parser.h"
#include "lexer.h"

int main(int argc, char *argv[]) {
    // yydebug = 1;
    NodePool pool = pool_initialize();
    ExprID root = NO_ID;

    yyscan_t scanner;
    if (yylex_init(&scanner)) {
        return 1;
    }

    // YY_BUFFER_STATE state = yy_scan_string(argv[1], scanner);

    if (yyparse(pool, &root, scanner)) {
        /* error parsing */
        return 1;
    }

    // yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    pool_display(pool, root, stdout);

    pool_release(pool);

    /*
    ExprID c0 = pool_int_constant(pool, 0);
    ExprID c1 = pool_int_constant(pool, 1);
    ExprID mul = pool_binary(pool, c0, c1, BinaryOp_MUL);
    ExprID add = pool_binary(pool, mul, c1, BinaryOp_ADD);
    ExprID expr = pool_binary(pool, add, pool_var(pool, "y"), BinaryOp_MUL);

    StmtID prog = pool_declaration(pool, NO_ID, Type_INT, "x");
    StmtID def = pool_assignment(pool, prog, "x", expr);
    StmtID ret1 = pool_ret(pool, def, pool_var(pool, "x"));
    pool_ret(pool, ret1, NO_ID);

    pool_display(pool, expr, stdout);
    pool_display(pool, prog, stdout);

    pool_release(pool);
    */
}

