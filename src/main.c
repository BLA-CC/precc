#include "ast.h"
#include "parser.h"
#include "lexer.h"
#include "str_pool.h"
#include "ast_visitor.h"
#include "sempass.h"

int main(int argc, char *argv[]) {
    // yydebug = 1;
    Ast ast = ast_initialize();
    StrPool strs = str_pool_init();
    NodeID root = NO_ID;

    yyscan_t scanner;
    if (yylex_init(&scanner)) {
        return 1;
    }
    yylex_init_extra(strs, scanner);

    // YY_BUFFER_STATE state = yy_scan_string(argv[1], scanner);

    if (yyparse(ast, &root, scanner)) {
        return 1;
    }

    // yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    ast_display(ast, root, strs, stdout);
    Status s = sempass(ast, root, strs);
    printf("Status: %d\n", s);

    ast_release(ast);
    str_pool_release(strs);
}
