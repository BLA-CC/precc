#include "../include/ast_visitor.h"
#include "../include/sym_table.h"
#include "../include/error.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: When we define blocks and functions these workarounds should go
#define MAIN_STR "main"
bool pending_return = false;

void error_msg(Status status, const char *detailed_msg) {
    char *error_msg;
    switch (status) {
    case Status_UndeclSymbol:
        error_msg = "Undeclared symbol";
        break;
    case Status_TypeError:
        error_msg = "Incompatible types";
        break;
    case Status_MultiDeclSymbol:
        error_msg = "Multiple declaration of the symbol";
        break;
    case Status_MissingReturn:
        error_msg = "Missing return expression";
        break;
    default:
        return;
    }

    fprintf(stderr, "Error: %s %s\n", error_msg, detailed_msg);
}

Status _tyck_int_constant(Visitor v, ExprID e_id) {
    ast_get_expr(visitor_get_ast(v), e_id)->type = Type_INT;
    return Status_OK;
}

Status _tyck_bool_constant(Visitor v, ExprID e_id) {
    ast_get_expr(visitor_get_ast(v), e_id)->type = Type_BOOL;
    return Status_OK;
}

Status _tyck_var(Visitor v, ExprID e_id) {
    Expression *e = ast_get_expr(visitor_get_ast(v), e_id);
    SymNode symnode = symtable_get_info(
        (SymTable)visitor_get_additional_args(v), e->data.var);

    if (symnode == NULL) {
        error_msg(
            Status_UndeclSymbol,
            str_pool_get(visitor_get_strs(v), e->data.var));
        return Status_UndeclSymbol;
    }

    e->type = symnode_get_symbol(symnode).type;
    return Status_OK;
}

Status _tyck_binary_expr(Visitor v, ExprID e_id) {
    AST ast = visitor_get_ast(v);
    Expression *expr = ast_get_expr(ast, e_id);

    Status s_l, s_r;
    if ((s_l = visit_expr(v, expr->data.binary.lhs)) != Status_OK)
        return s_l;
    if ((s_r = visit_expr(v, expr->data.binary.rhs)) != Status_OK)
        return s_r;

    if (ast_get_expr(ast, expr->data.binary.lhs)->type != Type_INT ||
        ast_get_expr(ast, expr->data.binary.rhs)->type != Type_INT) {
        error_msg(Status_TypeError, "in binary operation, int expected");
        return Status_TypeError;
    }

    expr->type = Type_INT;
    return Status_OK;
}

Status _tyck_declaration(Visitor v, StmtID s_id) {
    SymTable sym_table = (SymTable)visitor_get_additional_args(v);
    Statement *stmt = ast_get_stmt(visitor_get_ast(v), s_id);

    if (symtable_get_info(sym_table, stmt->data.declaration.ident) != NULL) {
        error_msg(
            Status_MultiDeclSymbol,
            str_pool_get(visitor_get_strs(v), stmt->data.declaration.ident));
        return Status_MultiDeclSymbol;
    }

    symtable_add_symbol(
        sym_table, stmt->data.declaration.ident, stmt->data.declaration.type);
    return Status_OK;
}

Status _tyck_assignment(Visitor v, StmtID s_id) {
    AST ast = visitor_get_ast(v);
    Statement *stmt = ast_get_stmt(ast, s_id);

    SymNode symnode = symtable_get_info(
        (SymTable)visitor_get_additional_args(v), stmt->data.assignment.ident);

    if (symnode == NULL) {
        error_msg(
            Status_UndeclSymbol,
            str_pool_get(visitor_get_strs(v), stmt->data.assignment.ident));
        return Status_UndeclSymbol;
    }

    Status s;
    if ((s = visit_expr(v, stmt->data.assignment.expr)) != Status_OK) {
        return s;
    }

    Type expr_type = ast_get_expr(ast, stmt->data.assignment.expr)->type;

    if (symnode_get_symbol(symnode).type != expr_type) {
        error_msg(Status_TypeError, "in assignment");
        return Status_TypeError;
    }

    return Status_OK;
}

Status _tyck_return(Visitor v, StmtID s_id) {
    pending_return = false;
    Type main_sym_type =
        symnode_get_symbol(symtable_get_info(
                               (SymTable)visitor_get_additional_args(v),
                               str_pool_put(visitor_get_strs(v), MAIN_STR)))
            .type;

    AST ast = visitor_get_ast(v);
    Statement *stmt = ast_get_stmt(ast, s_id);

    if (ast_get_expr(ast, stmt->data.ret_val) == NULL) {
        if (main_sym_type != Type_VOID) {
            error_msg(Status_MissingReturn, "");
            return Status_TypeError;
        }
        return Status_OK;
    }

    Status s;
    if ((s = visit_expr(v, stmt->data.ret_val)) != Status_OK) {
        return s;
    }

    if (ast_get_expr(ast, stmt->data.ret_val)->type != main_sym_type) {
        error_msg(Status_TypeError, "in return expression");
        return Status_TypeError;
    }

    return Status_OK;
}

Status _tyck_main(Visitor v, StmtID s_id) {
    Statement *stmt = ast_get_stmt(visitor_get_ast(v), s_id);
    pending_return = stmt->data.main.type != Type_VOID;

    symtable_add_symbol(
        (SymTable)visitor_get_additional_args(v),
        str_pool_put(visitor_get_strs(v), MAIN_STR),
        stmt->data.main.type);

    Status s;
    if ((s = visit_stmt(v, stmt->data.main.body)) != Status_OK) {
        return s;
    }

    if (pending_return) {
        error_msg(Status_MissingReturn, "");
        return Status_MissingReturn;
    }

    return Status_OK;
}

Status sempass(const AST ast, NodeID node_id, StrPool strs) {
    SymTable sym_table = symtable_initialize();

    Visitor visitor = init_visitor(
        ast,
        strs,
        sym_table,
        _tyck_int_constant,
        _tyck_bool_constant,
        _tyck_var,
        _tyck_binary_expr,
        _tyck_declaration,
        _tyck_assignment,
        _tyck_return,
        _tyck_main);

    Status status = ast_visit(visitor, node_id);

    symtable_release(sym_table);
    return status;
}
