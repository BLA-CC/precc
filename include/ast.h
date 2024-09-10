#ifndef _AST_H
#define _AST_H

#include <stdbool.h>
#include <stdio.h>

#include "defs.h"
#include "str_pool.h"

#define NO_ID UINT32_MAX

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef uint32_t NodeID;

typedef enum {
    BinOp_ADD,
    BinOp_MUL,
} BinOp;

#define FOR_AST_NODES(DO)                                   \
    /* expressions */                                       \
    DO(BOOL_CONSTANT, bool)                                 \
    DO(INT_CONSTANT, int64_t)                               \
    DO(BINOP, struct { NodeID lhs; NodeID rhs; BinOp op; }) \
    DO(VAR, StrID)                                          \
    /* statements */                                        \
    DO(DECL, struct { StrID var; Type type; })              \
    DO(ASGN, struct { StrID var; NodeID expr; })            \
    DO(RET, NodeID)                                         \
    /* toplevel */                                          \
    DO(MAIN, struct { NodeID body; Type ret_type; })        \

#define MK_KINDS(name, type) AstNodeKind_ ## name,
typedef enum {
    FOR_AST_NODES(MK_KINDS)
} AstNodeKind;
#undef MK_KINDS

#define MK_DATA(name, type) type name;
typedef union {
    FOR_AST_NODES(MK_DATA)
} AstNodeData;
#undef MK_DATA

typedef union {
    Type expr_type;
    NodeID stmt_next;
} AstNodeHeader;

typedef struct {
    AstNodeKind kind;
    AstNodeHeader header;
    AstNodeData data;
} AstNode;


struct AstPool_S {
    AstNode *data;
    size_t size;
    size_t capacity;
};

typedef struct AstPool_S *Ast;

/**
 * @brief Create and initialize the AST
 *
 * @returns A valid instance of AST if sucessful, NULL otherwise
 */
Ast ast_initialize();

/**
 * @brief Free the memory of the AST
 *
 * @note All IDs from the AST are considered invalid after calling this
 */
void ast_release(Ast self);

/**
 * @brief Push a 'return' Statement into the AST
 *
 * @param[in] prev - ID of a previous statement or NO_ID
 * @param[in] expr - ID of a valid expression or NO_ID
 *
 * @returns The ID of the new node if successful, NO_ID otherwise
 */
NodeID ast_mk_ret(Ast self, NodeID prev, NodeID expr);

/**
 * @brief Push a 'declaration' Statement into the AST
 *
 * @param[in] prev - ID of a previous statement or NO_ID
 * @param[in] type - Datatype of the variable to be declared
 * @param[in] ident - Name of the variable to be declared
 *
 * @returns The ID of the new node if successful, NO_ID otherwise
 */
NodeID ast_mk_decl(Ast self, NodeID prev, Type type, StrID ident);

/**
 * @brief Push an 'assignment' Statement into the AST
 *
 * @param[in] prev - ID of a previous statement or NO_ID
 * @param[in] ident - Name of the variable to be declared
 * @param[in] expr - ID of a valid node from the AST
 *
 * @returns The ID of the new node if successful, NO_ID otherwise
 */
NodeID ast_mk_asgn(Ast self, NodeID prev, StrID ident, NodeID expr);

/**
 * @brief Push a 'main' Statement into the AST
 *
 * @param[in] type - Return type
 * @param[in] body - ID of a valid node from the AST
 *
 * @returns The ID of the new node if successful, NO_ID otherwise
 */
NodeID ast_mk_main(Ast self, Type type, NodeID body);

/**
 * @brief Push an 'integer constant' Expression into the AST
 *
 * @param[in] constant - Value of the constant to push
 *
 * @returns The ID of the new node if successful, NO_ID otherwise
 */
NodeID ast_mk_int(Ast self, int64_t constant);

/**
 * @brief Push a 'boolean constant' Statement into the AST
 *
 * @param[in] constant - Value of the constant to push
 *
 * @returns The ID of the new node if successful, NO_ID otherwise
 */
NodeID ast_mk_bool(Ast self, bool constant);

/**
 * @brief Push a 'var' Statement into the AST
 *
 * @param[in] var - Name of the variable to push
 *
 * @returns The ID of the new node if successful, NO_ID otherwise
 */
NodeID ast_mk_var(Ast self, StrID var);

/**
 * @brief Push a 'binary' Statement into the AST
 *
 * @param[in] lhs - ID of a valid node from the AST
 * @param[in] rhs - ID of a valid node from the AST
 * @param[in] op - Binary operation between `lhs` and `rhs`
 *
 * @returns The ID of the new node if successful, NO_ID otherwise
 */
NodeID ast_mk_binop(Ast self, NodeID lhs, NodeID rhs, BinOp op);

/**
 * @brief Get a Statement from the AST
 *
 * @param[in] id - Valid ID of a Statement from the AST
 *
 * @returns A Statement if `id` represents a valid Statement, NULL otherwise
 */
AstNode *ast_get_stmt(const Ast self, NodeID id);

/**
 * @brief Get an Expression from the AST
 *
 * @param[in] id - Valid ID of an Expression from the AST
 *
 * @returns An Expression if `id` represents a valid Expression, NULL otherwise
 */
AstNode *ast_get_expr(const Ast self, NodeID id);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _AST_H */
