#include "ast.h"

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_CAPACITY 64

typedef enum {
    PoolEntryKind_Expression,
    PoolEntryKind_Statement,
} PoolEntryKind;

typedef union {
    Expression expr;
    Statement stmt;
} PoolEntryData;

typedef struct {
    PoolEntryKind kind;
    PoolEntryData data;
} PoolEntry;

struct NodePool_S {
    PoolEntry *entries;
    size_t size;
    size_t capacity;
};

//
// constructor & destructor
//

NodePool pool_initialize() {
    NodePool self = (NodePool)calloc(1, sizeof(*self));

    if (self == NULL) {
        return NULL;
    }

    return self;
}

void pool_release(NodePool self) {
    if (self == NULL) {
        return;
    }

    free(self->entries);
    free(self);
}

//
// tree construction
//

static inline bool _should_reallocate(NodePool self) {
    return self->size == self->capacity;
}

static bool _try_reallocate(NodePool self) {
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

static NodeID _push(NodePool self, const PoolEntry *entry) {
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

StmtID pool_ret(NodePool self, StmtID prev, ExprID expr) {
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

StmtID
pool_declaration(NodePool self, StmtID prev, Type type, StrID ident) {
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

StmtID
pool_assignment(NodePool self, StmtID prev, StrID ident, ExprID expr) {
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

ExprID pool_int_constant(NodePool self, int64_t constant) {
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

ExprID pool_bool_constant(NodePool self, bool constant) {
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

ExprID pool_var(NodePool self, StrID var) {
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

ExprID pool_binary(NodePool self, ExprID lhs, ExprID rhs, BinaryOp op) {
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

const Statement *pool_get_stmt(const NodePool self, StmtID id) {
    if (self->size <= id) {
        return NULL;
    }

    PoolEntry *entry = &self->entries[id];

    if (entry->kind != PoolEntryKind_Statement) {
        return NULL;
    }

    return &entry->data.stmt;
}

const Expression *pool_get_expr(const NodePool self, ExprID id) {
    if (self->size <= id) {
        return NULL;
    }

    PoolEntry *entry = &self->entries[id];

    if (entry->kind != PoolEntryKind_Expression) {
        return NULL;
    }

    return &entry->data.expr;
}

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

static void _pool_display_expr(const NodePool self, ExprID root, const StrPool strs, FILE *stream) {
    const Expression *expr = pool_get_expr(self, root);

    if (expr == NULL) {
        return;
    }

    switch (expr->kind) {
    case ExpressionKind_INT_CONSTANT:
        fprintf(stream, "%" PRIi64 "", expr->data.int_constant);
        break;

    case ExpressionKind_BOOL_CONSTANT:
        fprintf(stream, "%s", _str_bool(expr->data.bool_constant));
        break;

    case ExpressionKind_VAR:
        fprintf(stream, "%s", str_pool_get(strs, expr->data.var));
        break;

    case ExpressionKind_BINARY:
        fprintf(stream, "(");
        _pool_display_expr(self, expr->data.binary.lhs, strs, stream);
        fprintf(stream, " %s ", _str_bin_op(expr->data.binary.op));
        _pool_display_expr(self, expr->data.binary.rhs, strs, stream);
        fprintf(stream, ")");
        break;

    default:
        break;
    }
}

static void _pool_display_stmt(const NodePool self, StmtID root, StrPool strs, FILE *stream) {
    while (root != NO_ID) {
        const Statement *stmt = pool_get_stmt(self, root);

        if (stmt == NULL) {
            break;
        }

        switch (stmt->kind) {
        case StatementKind_DECLARATION:
            fprintf(
                stream,
                "%s %s",
                _str_type(stmt->data.declaration.type),
                str_pool_get(strs, stmt->data.declaration.ident));
            break;

        case StatementKind_ASSIGNMENT:
            fprintf(stream, "%s = ", str_pool_get(strs, stmt->data.assignment.ident));
            _pool_display_expr(self, stmt->data.assignment.expr, strs, stream);
            break;

        case StatementKind_RETURN:
            fprintf(stream, "return");
            if (stmt->data.ret_val != NO_ID) {
                fprintf(stream, " ");
                _pool_display_expr(self, stmt->data.ret_val, strs, stream);
            }
            break;

        default:
            break;
        }

        fprintf(stream, ";\n");

        root = stmt->next;
    }
}

void pool_display(const NodePool self, NodeID root, StrPool strs, FILE *stream) {
    if (stream == NULL || self->size <= root) {
        return;
    }

    switch (self->entries[root].kind) {
    case PoolEntryKind_Statement:
        _pool_display_stmt(self, root, strs, stream);
        break;

    case PoolEntryKind_Expression:
        _pool_display_expr(self, root, strs, stream);
        fprintf(stream, "\n");
        break;
    }
}
