#include "../include/ast.h"

#include <stdlib.h>
#include <string.h>

#define DEFAULT_CAPACITY 64

//
// constructor & destructor
//

Ast ast_initialize() {
    Ast self = (Ast)calloc(1, sizeof(*self));
    return self;
}

void ast_release(Ast self) {
    if (self == NULL) {
        return;
    }

    free(self->data);
    free(self);
}

//
// tree construction
//

static inline bool _should_reallocate(Ast self) {
    return self->size == self->capacity;
}

static bool _try_reallocate(Ast self) {
    AstNode *dummy = NULL;
    size_t new_capacity = 0;

    if (self->capacity == 0) {
        new_capacity = DEFAULT_CAPACITY;
        dummy = (AstNode *)calloc(new_capacity, sizeof(*dummy));
    } else {
        new_capacity = 2 * self->capacity;
        dummy =
            (AstNode *)realloc(self->data, new_capacity * sizeof(*dummy));
    }

    if (dummy == NULL) {
        return false;
    }

    self->capacity = new_capacity;
    self->data = dummy;

    return true;
}

static NodeID _push(Ast self, const AstNode *entry) {
    if (_should_reallocate(self)) {
        bool reallocate_success = _try_reallocate(self);

        if (!reallocate_success) {
            return NO_ID;
        }
    }

    memcpy(self->data + self->size, entry, sizeof(*entry));

    NodeID id = self->size;
    ++self->size;

    return id;
}

NodeID ast_mk_ret(Ast self, NodeID prev, NodeID expr) {
    AstNode entry = (AstNode){
        .kind = AstNodeKind_RET,
        .header = { .stmt_next = NO_ID },
        .data = { .RET = expr },
    };

    NodeID node_id = _push(self, &entry);

    if (prev != NO_ID) {
        self->data[prev].header.stmt_next = node_id;
    }

    return node_id;
}

NodeID ast_mk_decl(Ast self, NodeID prev, Type type, StrID ident) {
    AstNode entry = (AstNode){
        .kind = AstNodeKind_DECL,
        .header = { .stmt_next = NO_ID },
        .data = { .DECL = {
            .var = ident,
            .type = type,
        } },
    };

    NodeID node_id = _push(self, &entry);

    if (prev != NO_ID) {
        self->data[prev].header.stmt_next = node_id;
    }

    return node_id;
}

NodeID ast_mk_asgn(Ast self, NodeID prev, StrID ident, NodeID expr) {
    AstNode entry = (AstNode){
        .kind = AstNodeKind_ASGN,
        .header = { .stmt_next = NO_ID },
        .data = { .ASGN = {
            .var = ident,
            .expr = expr,
        } },
    };

    NodeID node_id = _push(self, &entry);

    if (prev != NO_ID) {
        self->data[prev].header.stmt_next = node_id;
    }

    return node_id;
}

NodeID ast_mk_main(Ast self, Type type, NodeID body) {
    AstNode entry = (AstNode){
        .kind = AstNodeKind_MAIN,
        .header = { .stmt_next = NO_ID },
        .data = { .MAIN = {
            .ret_type = type,
            .body = body,
        } },
    };

    NodeID node_id = _push(self, &entry);
    return node_id;
}

NodeID ast_mk_int(Ast self, int64_t constant) {
    AstNode entry = (AstNode){
        .kind = AstNodeKind_INT_CONSTANT,
        .header = {},
        .data = { .INT_CONSTANT = constant },
    };

    NodeID node_id = _push(self, &entry);
    return node_id;
}

NodeID ast_mk_bool(Ast self, bool constant) {
    AstNode entry = (AstNode){
        .kind = AstNodeKind_BOOL_CONSTANT,
        .header = {},
        .data = { .BOOL_CONSTANT = constant },
    };

    NodeID node_id = _push(self, &entry);
    return node_id;
}

NodeID ast_mk_var(Ast self, StrID var) {
    AstNode entry = (AstNode){
        .kind = AstNodeKind_VAR,
        .header = {},
        .data = { .VAR = var },
    };

    NodeID node_id = _push(self, &entry);
    return node_id;
}

NodeID ast_mk_binop(Ast self, NodeID lhs, NodeID rhs, BinOp op) {
    AstNode entry = (AstNode){
        .kind = AstNodeKind_BINOP,
        .header = {},
        .data = { .BINOP = {
            .op = op,
            .lhs = lhs,
            .rhs = rhs,
        } },
    };

    NodeID node_id = _push(self, &entry);
    return node_id;
}

//
// utility
//

AstNode *ast_get_stmt(const Ast self, NodeID id) {
    if (self->size <= id) {
        return NULL;
    }

    AstNode *entry = &self->data[id];

    if (entry->kind < AstNodeKind_DECL) {
        return NULL;
    }

    return entry;
}

AstNode *ast_get_expr(const Ast self, NodeID id) {
    if (self->size <= id) {
        return NULL;
    }

    AstNode *entry = &self->data[id];

    if (entry->kind >= AstNodeKind_DECL) {
        return NULL;
    }

    return entry;
}
