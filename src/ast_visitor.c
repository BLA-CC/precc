#include <stdio.h>
#include <stdlib.h>
#include "../include/ast.h"
#include "../include/str_pool.h"
#include "../include/ast_visitor.h"
#include "../include/error.h"

#include <inttypes.h>

// VISITOR
struct Visitor_S {
    Ast ast;
    StrPool strs;
    void *context;
    bool interrupt;

    /* Expression visitors */
    Status (*visit_int_constant)(Visitor visitor, NodeID expr_id);
    Status (*visit_bool_constant)(Visitor visitor, NodeID expr_id);
    Status (*visit_var)(Visitor visitor, NodeID expr_id);
    Status (*visit_binary_expr)(Visitor visitor, NodeID expr_id);

    /* Statement visitors */
    Status (*visit_declaration)(Visitor visitor, NodeID stmt_id);
    Status (*visit_assignment)(Visitor visitor, NodeID stmt_id);
    Status (*visit_return)(Visitor visitor, NodeID stmt_id);
    Status (*visit_main)(Visitor visitor, NodeID stmt_id);
};

Visitor init_visitor(
    Ast ast,
    StrPool strs,
    void *context,
    Status (*visit_int_constant)(Visitor visitor, NodeID expr_id),
    Status (*visit_bool_constant)(Visitor visitor, NodeID expr_id),
    Status (*visit_var)(Visitor visitor, NodeID expr_id),
    Status (*visit_binary_expr)(Visitor visitor, NodeID expr_id),
    Status (*visit_declaration)(Visitor visitor, NodeID stmt_id),
    Status (*visit_assignment)(Visitor visitor, NodeID stmt_id),
    Status (*visit_return)(Visitor visitor, NodeID stmt_id),
    Status (*visit_main)(Visitor visitor, NodeID stmt_id)) {
    // TODO: We could define some generic visits if some of the
    // pointers are NULL

    Visitor self = malloc(sizeof(*self));
    self->interrupt = false;
    self->ast = ast;
    self->strs = strs;
    self->context = context;
    self->visit_int_constant = visit_int_constant;
    self->visit_bool_constant = visit_bool_constant;
    self->visit_var = visit_var;
    self->visit_binary_expr = visit_binary_expr;
    self->visit_declaration = visit_declaration;
    self->visit_assignment = visit_assignment;
    self->visit_return = visit_return;
    self->visit_main = visit_main;

    return self;
}

Status visit_expr(Visitor self, NodeID expr_id) {
    if (self->interrupt) {
        return Status_OK;
    }

    AstNode *expr = ast_get_expr(self->ast, expr_id);
    if (expr == NULL) {
        return Status_OK;
    }

    Status s;
    switch (expr->kind) {
    case AstNodeKind_INT_CONSTANT:
        s = self->visit_int_constant(self, expr_id);
        break;

    case AstNodeKind_BOOL_CONSTANT:
        s = self->visit_bool_constant(self, expr_id);
        break;

    case AstNodeKind_VAR:
        s = self->visit_var(self, expr_id);
        break;

    case AstNodeKind_BINOP:
        s = self->visit_binary_expr(self, expr_id);
        break;

    case AstNodeKind_DECL:
    case AstNodeKind_ASGN:
    case AstNodeKind_RET:
    case AstNodeKind_MAIN:
        return Status_InternalError;
    }

    return s;
}

Status visit_stmt(Visitor self, NodeID stmt_id) {
    if (self->interrupt) {
        return Status_OK;
    }

    AstNode *stmt = ast_get_stmt(self->ast, stmt_id);
    if (stmt == NULL) {
        return Status_OK;
    }

    Status s;
    switch (stmt->kind) {
    case AstNodeKind_DECL:
        s = self->visit_declaration(self, stmt_id);
        break;

    case AstNodeKind_ASGN:
        s = self->visit_assignment(self, stmt_id);
        break;

    case AstNodeKind_RET:
        s = self->visit_return(self, stmt_id);
        break;

    case AstNodeKind_MAIN:
        s = self->visit_main(self, stmt_id);
        break;

    case AstNodeKind_BINOP:
    case AstNodeKind_VAR:
    case AstNodeKind_INT_CONSTANT:
    case AstNodeKind_BOOL_CONSTANT:
        return Status_InternalError;
    }

    Status next_s = Status_OK;
    if (stmt->header.stmt_next != NO_ID) {
        next_s = visit_stmt(self, stmt->header.stmt_next);
    }

    return s != Status_OK ? s : next_s;
}

Status ast_visit(Visitor self, NodeID node_id) {
    Ast ast = self->ast;

    if (ast->size <= node_id) {
        return Status_InternalError;
    }

    return visit_stmt(self, node_id);
}

void visitor_interrupt(Visitor self) {
    self->interrupt = true;
}

void visitor_release(Visitor self) {
    free(self);
}

// getter
void *visitor_get_context(Visitor self) {
    return self->context;
}

Ast visitor_get_ast(Visitor self) {
    return self->ast;
}

StrPool visitor_get_strs(Visitor self) {
    return self->strs;
}

//
// INSTANCES OF VISITORS
//

// DISPLAY AST

static const char *_str_bin_op(BinOp op) {
    switch (op) {
    case BinOp_ADD:
        return "+";
    case BinOp_MUL:
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

Status display_int_constant(Visitor visitor, NodeID expr_id) {
    FILE *stream = (FILE *)visitor->context;
    AstNode *expr = ast_get_expr(visitor->ast, expr_id);

    fprintf(stream, "%" PRIi64 "", expr->data.INT_CONSTANT);
    return 0;
}

Status display_bool_constant(Visitor visitor, NodeID expr_id) {
    FILE *stream = (FILE *)visitor->context;
    AstNode *expr = ast_get_expr(visitor->ast, expr_id);
    fprintf(stream, "%s", _str_bool(expr->data.BOOL_CONSTANT));

    return 0;
}

Status display_var(Visitor visitor, NodeID expr_id) {
    FILE *stream = (FILE *)visitor->context;
    AstNode *expr = ast_get_expr(visitor->ast, expr_id);

    fprintf(stream, "%s", str_pool_get(visitor->strs, expr->data.VAR));
    return 0;
}

Status display_binary_expr(Visitor visitor, NodeID expr_id) {
    FILE *stream = (FILE *)visitor->context;
    AstNode *expr = ast_get_expr(visitor->ast, expr_id);

    fprintf(stream, "(");
    visit_expr(visitor, expr->data.BINOP.lhs);
    fprintf(stream, " %s ", _str_bin_op(expr->data.BINOP.op));
    visit_expr(visitor, expr->data.BINOP.rhs);
    fprintf(stream, ")");
    return 0;
}

Status display_declaration(Visitor visitor, NodeID stmt_id) {
    FILE *stream = (FILE *)visitor->context;
    AstNode *stmt = ast_get_stmt(visitor->ast, stmt_id);

    fprintf(
        stream,
        "%s %s",
        _str_type(stmt->data.DECL.type),
        str_pool_get(visitor->strs, stmt->data.DECL.var));
    fprintf(stream, ";\n");
    return 0;
}

Status display_assignment(Visitor visitor, NodeID stmt_id) {
    FILE *stream = (FILE *)visitor->context;
    AstNode *stmt = ast_get_stmt(visitor->ast, stmt_id);

    fprintf(stream, "%s = ", str_pool_get(visitor->strs, stmt->data.ASGN.var));
    visit_expr(visitor, stmt->data.ASGN.expr);
    fprintf(stream, ";\n");
    return 0;
}

Status display_return(Visitor visitor, NodeID stmt_id) {
    FILE *stream = (FILE *)visitor->context;
    AstNode *stmt = ast_get_stmt(visitor->ast, stmt_id);

    fprintf(stream, "return");
    if (stmt->data.RET != NO_ID) {
        fprintf(stream, " ");
        visit_expr(visitor, stmt->data.RET);
    }
    fprintf(stream, ";\n");
    return 0;
}

Status display_main(Visitor visitor, NodeID stmt_id) {
    FILE *stream = (FILE *)visitor->context;
    AstNode *stmt = ast_get_stmt(visitor->ast, stmt_id);

    fprintf(stream, "%s main() {\n", _str_type(stmt->data.MAIN.ret_type));
    NodeID body = stmt->data.MAIN.body;
    if (body != NO_ID) {
        visit_stmt(visitor, stmt->data.MAIN.body);
    }
    fprintf(stream, "}");
    return 0;
}

void ast_display(const Ast ast, NodeID node_id, StrPool strs, FILE *stream) {

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
