#include "../include/ast.h"

#include <stdlib.h>
#include <string.h>

#define DEFAULT_CAPACITY 64

//
// constructor & destructor
//

AST ast_initialize() {
    AST self = (AST)calloc(1, sizeof(*self));
    return self;
}

void ast_release(AST self) {
    if (self == NULL) {
        return;
    }

    free(self->entries);
    free(self);
}

//
// tree construction
//

static inline bool _should_reallocate(AST self) {
    return self->size == self->capacity;
}

static bool _try_reallocate(AST self) {
    PoolEntry *dummy = NULL;
    size_t new_capacity = 0;

    if (self->capacity == 0) {
        new_capacity = DEFAULT_CAPACITY;
        dummy = (PoolEntry *)calloc(new_capacity, sizeof(*dummy));
    } else {
        new_capacity = 2 * self->capacity;
        dummy =
            (PoolEntry *)realloc(self->entries, new_capacity * sizeof(*dummy));
    }

    if (dummy == NULL) {
        return false;
    }

    self->capacity = new_capacity;
    self->entries = dummy;

    return true;
}

static NodeID _push(AST self, const PoolEntry *entry) {
    if (_should_reallocate(self)) {
        bool reallocate_success = _try_reallocate(self);

        if (!reallocate_success) {
            return NO_ID;
        }
    }

    memcpy(self->entries + self->size, entry, sizeof(*entry));

    NodeID id = self->size;
    ++self->size;

    return id;
}

StmtID ast_ret(AST self, StmtID prev, ExprID expr) {
    PoolEntry entry = (PoolEntry){
        .kind = PoolEntryKind_Statement,
        .data = { .stmt = {
            .kind = StatementKind_RETURN,
            .data = { .ret_val = expr },
            .next = NO_ID,
        } },
    };

    NodeID node_id = _push(self, &entry);

    if (prev != NO_ID) {
        self->entries[prev].data.stmt.next = node_id;
    }

    return node_id;
}

StmtID ast_declaration(AST self, StmtID prev, Type type, StrID ident) {
    PoolEntry entry = (PoolEntry){
        .kind = PoolEntryKind_Statement,
        .data = { .stmt = {
            .kind = StatementKind_DECLARATION,
            .data = { .declaration = {
                .ident = ident,
                .type = type,
            } },
            .next = NO_ID,
        } },
    };

    NodeID node_id = _push(self, &entry);

    if (prev != NO_ID) {
        self->entries[prev].data.stmt.next = node_id;
    }

    return node_id;
}

StmtID ast_assignment(AST self, StmtID prev, StrID ident, ExprID expr) {
    PoolEntry entry = (PoolEntry){
        .kind = PoolEntryKind_Statement,
        .data = { .stmt = {
            .kind = StatementKind_ASSIGNMENT,
            .data = { .assignment = {
                .ident = ident,
                .expr = expr,
            } },
            .next = NO_ID,
        } },
    };

    NodeID node_id = _push(self, &entry);

    if (prev != NO_ID) {
        self->entries[prev].data.stmt.next = node_id;
    }

    return node_id;
}

StmtID ast_main(AST self, Type type, StmtID body) {
    PoolEntry entry = (PoolEntry){
        .kind = PoolEntryKind_Statement,
        .data = { .stmt = {
            .kind = StatementKind_MAIN,
            .data = { .main = {
                .type = type,
                .body = body,
            } },
            .next = NO_ID,
        } },
    };

    NodeID node_id = _push(self, &entry);
    return node_id;
}

ExprID ast_int_constant(AST self, int64_t constant) {
    PoolEntry entry = (PoolEntry){
        .kind = PoolEntryKind_Expression,
        .data = { .expr = {
            .kind = ExpressionKind_INT_CONSTANT,
            .data = { .int_constant = constant },
        } },
    };

    NodeID node_id = _push(self, &entry);
    return node_id;
}

ExprID ast_bool_constant(AST self, bool constant) {
    PoolEntry entry = (PoolEntry){
        .kind = PoolEntryKind_Expression,
        .data = { .expr = {
            .kind = ExpressionKind_BOOL_CONSTANT,
            .data = { .bool_constant = constant },
        } },
    };

    NodeID node_id = _push(self, &entry);
    return node_id;
}

ExprID ast_var(AST self, StrID var) {
    PoolEntry entry = (PoolEntry){
        .kind = PoolEntryKind_Expression,
        .data = { .expr = {
            .kind = ExpressionKind_VAR,
            .data = { .var = var },
        } },
    };

    NodeID node_id = _push(self, &entry);
    return node_id;
}

ExprID ast_binary(AST self, ExprID lhs, ExprID rhs, BinaryOp op) {
    PoolEntry entry = (PoolEntry){
        .kind = PoolEntryKind_Expression,
        .data = { .expr = {
            .kind = ExpressionKind_BINARY,
            .data = { .binary = {
                .op = op,
                .lhs = lhs,
                .rhs = rhs,
            } },
        } },
    };

    NodeID node_id = _push(self, &entry);
    return node_id;
}

//
// utility
//

Statement *ast_get_stmt(const AST self, StmtID id) {
    if (self->size <= id) {
        return NULL;
    }

    PoolEntry *entry = &self->entries[id];

    if (entry->kind != PoolEntryKind_Statement) {
        return NULL;
    }

    return &entry->data.stmt;
}

Expression *ast_get_expr(const AST self, ExprID id) {
    if (self->size <= id) {
        return NULL;
    }

    PoolEntry *entry = &self->entries[id];

    if (entry->kind != PoolEntryKind_Expression) {
        return NULL;
    }

    return &entry->data.expr;
}
