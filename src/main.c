#include "ast.h"
#include "parser.h"
#include "lexer.h"
#include "str_pool.h"

int main(int argc, char *argv[]) {
    // yydebug = 1;
    NodePool pool = pool_initialize();
    StrPool strs = str_pool_init();
    ExprID root = NO_ID;

    yyscan_t scanner;
    if (yylex_init(&scanner)) {
        return 1;
    }
    yylex_init_extra(strs, scanner);

    // YY_BUFFER_STATE state = yy_scan_string(argv[1], scanner);

    if (yyparse(pool, &root, scanner)) {
        return 1;
    }

    // yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    pool_display(pool, root, strs, stdout);

    pool_release(pool);
    str_pool_release(strs);
}

