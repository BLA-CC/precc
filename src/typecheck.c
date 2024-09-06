#include "../include/ast_visitor.h"
#include "../include/sym_table.h"


Status tyck_int_constant(Visitor v, ExprID e_id){
    Expression *e = ast_get_expr(visitor_get_ast(v), e_id);
    e->type = Type_INT;
    return 0;
}

Status tyck_bool_constant(Visitor v, ExprID e_id){
    Expression *e = ast_get_expr(visitor_get_ast(v), e_id);
    e->type = Type_BOOL;
    return 0;
}

Status tyck_var(Visitor v, ExprID e_id){
    SymTable sym_table = (SymTable)visitor_get_additional_args(v);
    Expression *e = ast_get_expr(visitor_get_ast(v), e_id);

    SymNode symnode = symtable_get_info(sym_table, e->data.var);
    
    if (symnode != NULL) {
        e->type = symnode_get_symbol(symnode).type;
    }

    return 0;
}

Status tyck_binary_expr(Visitor v, ExprID e_id) {
    SymTable sym_table = (SymTable)visitor_get_additional_args(v);
    AST ast = visitor_get_ast(v);
    Expression *expr = ast_get_expr(ast, e_id);
    expr->type = Type_INT;

    visit_expr(v, expr->data.binary.lhs);
    visit_expr(v, expr->data.binary.rhs);

    Type type_lh = ast_get_expr(ast, expr->data.binary.lhs)->type;
    Type type_rh = ast_get_expr(ast, expr->data.binary.rhs)->type;

    return !((type_lh == type_rh) && (type_lh == expr->type)); // FIXME: Que horror
}

Status tyck_declaration(Visitor v, StmtID s_id){
    SymTable sym_table = (SymTable)visitor_get_additional_args(v);
    AST ast = visitor_get_ast(v);
    Statement *stmt = ast_get_stmt(ast, s_id);
    StrID var_ident = stmt->data.declaration.ident;

    if (symtable_get_info(sym_table, var_ident) != NULL) {
        return 1; // Variable already declared
    }

    symtable_add_symbol(sym_table, var_ident, stmt->data.declaration.type);

    // TODO: hace falta anotar el tipo de la variable?

    return 0;
}

Status tyck_assignment(Visitor v, StmtID s_id) {
    AST ast = visitor_get_ast(v);
    Statement * stmt = ast_get_stmt(ast, s_id);
    SymTable sym_table = (SymTable)visitor_get_additional_args(v);

    SymNode symnode = symtable_get_info(sym_table, stmt->data.assignment.ident);
    Sym symbol = symnode_get_symbol(symnode);
    
    visit_expr(v, stmt->data.assignment.expr);
    Expression *expr_assgn = ast_get_expr(ast,stmt->data.assignment.expr);
    if (symbol.type == expr_assgn->type) {
        return 0; // FIXME : handleo de errores
    }
    return 1;
}

Status tyck_return(Visitor v, StmtID s_id) {
    return 0;
}

Status sempass(const AST ast, NodeID node_id, StrPool strs) {
    SymTable sym_table = symtable_initialize();

    Visitor visitor = init_visitor(
        ast, strs, sym_table, tyck_int_constant,
        tyck_bool_constant, tyck_var, tyck_binary_expr,
        tyck_declaration, tyck_assignment, tyck_return
    );

    ast_visit(visitor, node_id);

    symtable_release(sym_table);
}
