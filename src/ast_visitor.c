#include <stdio.h>
#include <stdlib.h>
#include "../include/ast.h"
#include "../include/str_pool.h"
#include "../include/ast_visitor.h"
#include "../include/error.h"

#include <inttypes.h>

// VISITOR
struct Visitor_S {
    AST ast;
    StrPool strs;
    void *additional_args;

    /* Expression visitors */
    Status (*visit_int_constant)(Visitor visitor, ExprID expr_id);
    Status (*visit_bool_constant)(Visitor visitor, ExprID expr_id);
    Status (*visit_var)(Visitor visitor, ExprID expr_id);
    Status (*visit_binary_expr)(Visitor visitor, ExprID expr_id);

    /* Statement visitors */
    Status (*visit_declaration)(Visitor visitor, StmtID stmt_id);
    Status (*visit_assignment)(Visitor visitor, StmtID stmt_id);
    Status (*visit_return)(Visitor visitor, StmtID stmt_id);
    Status (*visit_main)(Visitor visitor, StmtID stmt_id);
};

Visitor init_visitor(
    AST ast,
    StrPool strs,
    void *additional_args,
    Status (*visit_int_constant)(Visitor visitor, ExprID expr_id),
    Status (*visit_bool_constant)(Visitor visitor, ExprID expr_id),
    Status (*visit_var)(Visitor visitor, ExprID expr_id),
    Status (*visit_binary_expr)(Visitor visitor, ExprID expr_id),
    Status (*visit_declaration)(Visitor visitor, StmtID stmt_id),
    Status (*visit_assignment)(Visitor visitor, StmtID stmt_id),
    Status (*visit_return)(Visitor visitor, StmtID stmt_id),
    Status (*visit_main)(Visitor visitor, StmtID stmt_id)) {
    // TODO: We could define some generic visits if some of the
    // pointers are NULL
    Visitor visitor = malloc(sizeof(struct Visitor_S));
    visitor->ast = ast;
    visitor->strs = strs;
    visitor->additional_args = additional_args;
    visitor->visit_int_constant = visit_int_constant;
    visitor->visit_bool_constant = visit_bool_constant;
    visitor->visit_var = visit_var;
    visitor->visit_binary_expr = visit_binary_expr;
    visitor->visit_declaration = visit_declaration;
    visitor->visit_assignment = visit_assignment;
    visitor->visit_return = visit_return;
    visitor->visit_main = visit_main;
    return visitor;
}

Status visit_expr(Visitor self, ExprID expr_id) {
    Expression *expr = ast_get_expr(self->ast, expr_id);
    Status s;

    switch (expr->kind) {
    case ExpressionKind_INT_CONSTANT:
        s = self->visit_int_constant(self, expr_id);
        break;

    case ExpressionKind_BOOL_CONSTANT:
        s = self->visit_bool_constant(self, expr_id);
        break;

    case ExpressionKind_VAR:
        s = self->visit_var(self, expr_id);
        break;

    case ExpressionKind_BINARY:
        s = self->visit_binary_expr(self, expr_id);
        break;
    }

    return s;
}

Status visit_stmt(Visitor self, StmtID stmt_id) {
    Statement *stmt = ast_get_stmt(self->ast, stmt_id);

    if (stmt == NULL) {
        return Status_OK;
    }

    Status s;
    switch (stmt->kind) {
    case StatementKind_DECLARATION:
        s = self->visit_declaration(self, stmt_id);
        break;

    case StatementKind_ASSIGNMENT:
        s = self->visit_assignment(self, stmt_id);
        break;

    case StatementKind_RETURN:
        s = self->visit_return(self, stmt_id);
        break;

    case StatementKind_MAIN:
        s = self->visit_main(self, stmt_id);
        break;
    }

    Status next_s = Status_OK;
    if (stmt->next != NO_ID) {
        next_s = visit_stmt(self, stmt->next);
    }

    return s != Status_OK ? s : next_s;
}

Status ast_visit(Visitor self, NodeID node_id) {
    AST ast = self->ast;

    if (ast->size <= node_id) {
        return Status_InternalError;
    }

    return visit_stmt(self, node_id);
}

void visitor_release(Visitor self) {
    free(self);
}

// getter
void *visitor_get_additional_args(Visitor self) {
    return self->additional_args;
}

AST visitor_get_ast(Visitor self) {
    return self->ast;
}

StrPool visitor_get_strs(Visitor self) {
    return self->strs;
}

//
// INSTANCES OF VISITORS
//

// DISPLAY AST

static const char *_str_bin_op(BinaryOp op) {
    switch (op) {
    case BinaryOp_ADD:
        return "+";
    case BinaryOp_MUL:
        return "*";
    default:
        return "INVALID BinaryOp";
    }
}

static const char *_str_type(Type type) {
    switch (type) {
    case Type_VOID:
        return "void";
    case Type_INT:
        return "int";
    case Type_BOOL:
        return "bool";
    default:
        return "INVALID Type";
    }
}

static const char *_str_bool(bool val) {
    if (val) {
        return "true";
    } else {
        return "false";
    }
}

Status display_int_constant(Visitor visitor, ExprID expr_id) {
    FILE *stream = (FILE *)visitor->additional_args;
    Expression *expr = ast_get_expr(visitor->ast, expr_id);

    fprintf(stream, "%" PRIi64 "", expr->data.int_constant);
    return 0;
}

Status display_bool_constant(Visitor visitor, ExprID expr_id) {
    FILE *stream = (FILE *)visitor->additional_args;
    Expression *expr = ast_get_expr(visitor->ast, expr_id);
    fprintf(stream, "%s", _str_bool(expr->data.bool_constant));

    return 0;
}

Status display_var(Visitor visitor, ExprID expr_id) {
    FILE *stream = (FILE *)visitor->additional_args;
    Expression *expr = ast_get_expr(visitor->ast, expr_id);

    fprintf(stream, "%s", str_pool_get(visitor->strs, expr->data.var));
    return 0;
}

Status display_binary_expr(Visitor visitor, ExprID expr_id) {
    FILE *stream = (FILE *)visitor->additional_args;
    Expression *expr = ast_get_expr(visitor->ast, expr_id);

    fprintf(stream, "(");
    visit_expr(visitor, expr->data.binary.lhs);
    fprintf(stream, " %s ", _str_bin_op(expr->data.binary.op));
    visit_expr(visitor, expr->data.binary.rhs);
    fprintf(stream, ")");
    return 0;
}

Status display_declaration(Visitor visitor, StmtID stmt_id) {
    FILE *stream = (FILE *)visitor->additional_args;
    Statement *stmt = ast_get_stmt(visitor->ast, stmt_id);

    fprintf(
        stream,
        "%s %s",
        _str_type(stmt->data.declaration.type),
        str_pool_get(visitor->strs, stmt->data.declaration.ident));
    fprintf(stream, ";\n");
    return 0;
}

Status display_assignment(Visitor visitor, StmtID stmt_id) {
    FILE *stream = (FILE *)visitor->additional_args;
    Statement *stmt = ast_get_stmt(visitor->ast, stmt_id);

    fprintf(
        stream,
        "%s = ",
        str_pool_get(visitor->strs, stmt->data.assignment.ident));
    visit_expr(visitor, stmt->data.assignment.expr);
    fprintf(stream, ";\n");
    return 0;
}

Status display_return(Visitor visitor, StmtID stmt_id) {
    FILE *stream = (FILE *)visitor->additional_args;
    Statement *stmt = ast_get_stmt(visitor->ast, stmt_id);

    fprintf(stream, "return");
    if (stmt->data.ret_val != NO_ID) {
        fprintf(stream, " ");
        visit_expr(visitor, stmt->data.ret_val);
    }
    fprintf(stream, ";\n");
    return 0;
}

Status display_main(Visitor visitor, StmtID stmt_id) {
    FILE *stream = (FILE *)visitor->additional_args;
    Statement *stmt = ast_get_stmt(visitor->ast, stmt_id);

    fprintf(stream, "%s main() {\n", _str_type(stmt->data.main.type));
    StmtID body = stmt->data.main.body;
    if (body != NO_ID) {
        visit_stmt(visitor, stmt->data.main.body);
    }
    fprintf(stream, "}");
    return 0;
}

void ast_display(const AST ast, NodeID node_id, StrPool strs, FILE *stream) {

    Visitor visitor = init_visitor(
        ast,
        strs,
        (void *)stream,
        display_int_constant,
        display_bool_constant,
        display_var,
        display_binary_expr,
        display_declaration,
        display_assignment,
        display_return,
        display_main);

    ast_visit(visitor, node_id);
    fprintf(stream, "\n");

    visitor_release(visitor);
}
