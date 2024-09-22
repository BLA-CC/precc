#include "interp.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "ast_visitor.h"
#include "defs.h"
#include "error.h"
#include "str_pool.h"
#include "sym_table.h"

const Sym VOID_SYM = { .ident = NO_ID, .type = Type_VOID };

typedef struct {
    StrPool strs;
    SymTable syms;
    Sym last_symbol;
} Context;

static Status _interp_int_constant(Visitor visitor, NodeID id) {
    Context *ctx = visitor_get_context(visitor);
    Ast ast = visitor_get_ast(visitor);

    const AstNode *node = ast_get_expr(ast, id);
    assert(node != NULL);

    ctx->last_symbol.ident = NO_ID;
    ctx->last_symbol.type = Type_INT;
    ctx->last_symbol.value.v_int = node->data.INT_CONSTANT;

    printf("INT: %" PRIi64 "\n", ctx->last_symbol.value.v_int);

    return Status_OK;
}

static Status _interp_bool_constant(Visitor visitor, NodeID id) {
    Context *ctx = visitor_get_context(visitor);
    Ast ast = visitor_get_ast(visitor);

    const AstNode *node = ast_get_expr(ast, id);
    assert(node != NULL);

    ctx->last_symbol.ident = NO_ID;
    ctx->last_symbol.type = Type_BOOL;
    ctx->last_symbol.value.v_bool = node->data.BOOL_CONSTANT;

    printf("BOOL: %d\n", ctx->last_symbol.value.v_bool);

    return Status_OK;
}

static Status _interp_var(Visitor visitor, NodeID id) {
    Context *ctx = visitor_get_context(visitor);
    Ast ast = visitor_get_ast(visitor);

    const AstNode *node = ast_get_expr(ast, id);
    assert(node != NULL);

    const Sym *sym =
        symnode_get_symbol(symtable_get_info(ctx->syms, node->data.VAR));
    assert(sym != NULL);

    printf(
        "VAR %d: %" PRIi64 "\n",
        ctx->last_symbol.type,
        ctx->last_symbol.value.v_int);

    memcpy(&ctx->last_symbol, sym, sizeof(*sym));

    return Status_OK;
}

static Status _interp_binary(Visitor visitor, NodeID id) {
    Context *ctx = visitor_get_context(visitor);
    Ast ast = visitor_get_ast(visitor);

    const AstNode *node = ast_get_expr(ast, id);
    assert(node != NULL);

    Status lhs_res = visit_expr(visitor, node->data.BINOP.lhs);
    (void)lhs_res; // TODO: handle error
    Sym lhs = ctx->last_symbol;

    Status rhs_res = visit_expr(visitor, node->data.BINOP.rhs);
    (void)rhs_res; // TODO: handle error
    Sym rhs = ctx->last_symbol;

    switch (node->data.BINOP.op) {
    // TODO: maybe define semantics for binary operations between Symbols
    case BinOp_ADD:
        ctx->last_symbol.value.v_int = lhs.value.v_int + rhs.value.v_int;
        break;

    case BinOp_MUL:
        ctx->last_symbol.value.v_int = lhs.value.v_int * rhs.value.v_int;
        break;
    }

    ctx->last_symbol.ident = NO_ID;
    ctx->last_symbol.type = Type_INT;

    return Status_OK;
}

static Status _interp_declaration(Visitor visitor, NodeID id) {
    Context *ctx = visitor_get_context(visitor);
    Ast ast = visitor_get_ast(visitor);

    const AstNode *node = ast_get_stmt(ast, id);
    assert(node != NULL);

    StrID ident = node->data.DECL.var;
    Type type = node->data.DECL.type;
    symtable_add_symbol(ctx->syms, ident, type);

    ctx->last_symbol = VOID_SYM;

    return Status_OK;
}

static Status _interp_assignment(Visitor visitor, NodeID id) {
    Context *ctx = visitor_get_context(visitor);
    Ast ast = visitor_get_ast(visitor);

    const AstNode *node = ast_get_stmt(ast, id);
    assert(node != NULL);

    Status expr_res = visit_expr(visitor, node->data.ASGN.expr);
    (void)expr_res; // TODO: handle error

    Sym expr = ctx->last_symbol;
    StrID ident = node->data.ASGN.var;

    Sym *sym_entry = symnode_get_symbol(symtable_get_info(ctx->syms, ident));
    assert(sym_entry != NULL);
    sym_entry->value = expr.value;

    ctx->last_symbol = VOID_SYM;

    return Status_OK;
}

static Status _interp_return(Visitor visitor, NodeID id) {
    Context *ctx = visitor_get_context(visitor);
    Ast ast = visitor_get_ast(visitor);

    const AstNode *node = ast_get_stmt(ast, id);
    assert(node != NULL);

    if (node->data.RET == NO_ID) {
        ctx->last_symbol = VOID_SYM;
    } else {
        Status ret_expr_res = visit_expr(visitor, node->data.RET);
        (void)ret_expr_res; // TODO: handle error
        // ctx->last_symbol is the return value
    }

    visitor_interrupt(visitor);

    return Status_OK;
}

static Status _interp_main(Visitor visitor, NodeID id) {
    Ast ast = visitor_get_ast(visitor);

    const AstNode *current_stmt = ast_get_stmt(ast, id);

    Status s = visit_stmt(visitor, current_stmt->data.MAIN.body);

    return s;
}

Sym interp(Ast ast, NodeID root, StrPool strs, SymTable syms) {
    Context ctx = { 0 };
    ctx.strs = strs;
    ctx.syms = syms;

    Visitor visitor = init_visitor(
        ast,
        strs,
        &ctx,
        _interp_int_constant,
        _interp_bool_constant,
        _interp_var,
        _interp_binary,
        _interp_declaration,
        _interp_assignment,
        _interp_return,
        _interp_main);

    Status status = ast_visit(visitor, root);
    (void)status; // TODO: handle error

    visitor_release(visitor);

    return ctx.last_symbol;
}
