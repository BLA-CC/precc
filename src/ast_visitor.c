#include <stdlib.h>
#include "ast.h"
#include "str_pool.h"
#include "ast_visitor.h"

#include <inttypes.h>

// VISITOR 
struct Visitor_S {
    AST ast;
    StrPool strs;
    void *additional_args;

    /* Expression visitors */
    Status  (*visit_int_constant)(Visitor visitor, Expression* expr);
    Status (*visit_bool_constant)(Visitor visitor, Expression* expr);
    Status           (*visit_var)(Visitor visitor, Expression* expr);
    Status   (*visit_binary_expr)(Visitor visitor, Expression* expr);

    /* Statement visitors */
    Status (*visit_declaration)(Visitor visitor, Statement* stmt);
    Status  (*visit_assignment)(Visitor visitor, Statement* stmt);
    Status      (*visit_return)(Visitor visitor, Statement* stmt);
};


Visitor init_visitor(
    AST ast,
    StrPool strs,
    void *additional_args,
    Status  (*visit_int_constant)(Visitor visitor, Expression* expr),
    Status (*visit_bool_constant)(Visitor visitor, Expression* expr),
    Status           (*visit_var)(Visitor visitor, Expression* expr),
    Status   (*visit_binary_expr)(Visitor visitor, Expression* expr),
    Status   (*visit_declaration)(Visitor visitor,  Statement* stmt),
    Status    (*visit_assignment)(Visitor visitor,  Statement* stmt),
    Status        (*visit_return)(Visitor visitor,  Statement* stmt)
){
    // TODO: We could define some generic visits if some of the 
    // pointers are NULL
    // FIXME: Esto es bastante horrible
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
    return visitor;
}

void visit_expr(Visitor self, Expression *expr) {
    switch (expr->kind) {
    case ExpressionKind_INT_CONSTANT:
        self->visit_int_constant(self, expr);
        break;

    case ExpressionKind_BOOL_CONSTANT:
        self->visit_bool_constant(self, expr);
        break;

    case ExpressionKind_VAR:
        self->visit_var(self, expr);
        break;

    case ExpressionKind_BINARY:
        self->visit_binary_expr(self, expr);
        break;

    default:
        break;
    }
}

void visit_stmt(Visitor self, Statement *stmt) {
    switch (stmt->kind) {
    case StatementKind_DECLARATION: 
        self->visit_declaration(self, stmt);
        break;

    case StatementKind_ASSIGNMENT:
        self->visit_assignment(self, stmt);
        break;

    case StatementKind_RETURN:
        self->visit_return(self, stmt);
        break;

    default:
        break;
    }

}

void ast_visit(Visitor self, NodeID node_id) {
    AST ast = self->ast;

    if (ast->size <= node_id) {
        return;
    }

    switch (ast->entries[node_id].kind) {
    case PoolEntryKind_Statement:

        while (node_id != NO_ID) {
            Statement *stmt = ast_get_stmt(ast, node_id);

            if (stmt == NULL) {
                break;
            }

            visit_stmt(self, stmt);

            node_id = stmt->next;
        }

        break;

    case PoolEntryKind_Expression:
        Expression *expr = ast_get_expr(ast, node_id);

        if (expr == NULL) {
            return;
        }

        visit_expr(self, expr);
        break;
    }
}


void visitor_release(Visitor self) {
    free(self);
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

Status display_int_constant(Visitor visitor, Expression* expr) {
    FILE *stream = (FILE *)visitor->additional_args;
    fprintf(stream, "%" PRIi64 "", expr->data.int_constant);
    return 0;
}

Status display_bool_constant(Visitor visitor, Expression* expr) {
    FILE *stream = (FILE *)visitor->additional_args;
    fprintf(stream, "%s", _str_bool(expr->data.bool_constant));
    return 0;
}


Status display_var(Visitor visitor, Expression* expr) {
    FILE *stream = (FILE *)visitor->additional_args;
    fprintf(stream, "%s", str_pool_get(visitor->strs, expr->data.var));
    return 0;
}

Status display_binary_expr(Visitor visitor, Expression* expr) {
    FILE *stream = (FILE *)visitor->additional_args;
    Expression *lhs_expression = ast_get_expr(visitor->ast, expr->data.binary.lhs);
    Expression *rhs_expression = ast_get_expr(visitor->ast, expr->data.binary.rhs);

    fprintf(stream, "(");
    visit_expr(visitor, lhs_expression);
    fprintf(stream, " %s ", _str_bin_op(expr->data.binary.op));
    visit_expr(visitor, rhs_expression);
    fprintf(stream, ")");
    return 0;
}

Status display_declaration(Visitor visitor, Statement *stmt) {
    FILE *stream = (FILE *)visitor->additional_args;

    fprintf(
        stream, "%s %s", 
        _str_type(stmt->data.declaration.type),
        str_pool_get(visitor->strs, stmt->data.declaration.ident)
    );
    fprintf(stream, ";\n");
    return 0;
}

Status display_assignment(Visitor visitor, Statement *stmt) {
    FILE *stream = (FILE *)visitor->additional_args;
    Expression *assignment_expr = ast_get_expr(visitor->ast, stmt->data.assignment.expr);
    fprintf(stream, "%s = ", str_pool_get(visitor->strs, stmt->data.assignment.ident));
    visit_expr(visitor, assignment_expr);
    fprintf(stream, ";\n");
    return 0;
}

Status display_return(Visitor visitor, Statement *stmt) {
    FILE *stream = (FILE *)visitor->additional_args;

    fprintf(stream, "return");
    if (stmt->data.ret_val != NO_ID) {
        Expression *return_expr = ast_get_expr(visitor->ast, stmt->data.ret_val);
        fprintf(stream, " ");
        visit_expr(visitor, return_expr);
    }
    fprintf(stream, ";\n");
    return 0;
}

void ast_display(const AST ast, NodeID node_id, StrPool strs, FILE *stream) {

    Visitor visitor = init_visitor(
        ast, strs, (void*)stream, display_int_constant,
        display_bool_constant, display_var, display_binary_expr,
        display_declaration, display_assignment, display_return
    );

    ast_visit(visitor, node_id);
    fprintf(stream, "\n");

    visitor_release(visitor);
}
