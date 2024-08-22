#ifndef _AST_H
#define _AST_H

#include <stdbool.h>
#include <stdio.h>

#include "defs.h"

#define NO_ID UINT32_MAX

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef uint32_t NodeID;
typedef uint32_t ExprID;
typedef uint32_t StmtID;

typedef enum {
    BinaryOp_ADD,
    BinaryOp_MUL,
} BinaryOp;

typedef enum {
    ExpressionKind_BOOL_CONSTANT,
    ExpressionKind_INT_CONSTANT,
    ExpressionKind_BINARY,
    ExpressionKind_VAR,
} ExpressionKind;

typedef union {
    char *var;
    bool bool_constant;
    int64_t int_constant;

    struct {
        ExprID lhs;
        ExprID rhs;
        BinaryOp op;
    } binary;
} ExpressionData;

typedef struct {
    ExpressionKind kind;
    ExpressionData data;
} Expression;

typedef enum {
    Type_VOID,
    Type_INT,
    Type_BOOL,
} Type;

typedef enum {
    StatementKind_DECLARATION,
    StatementKind_ASSIGNMENT,
    StatementKind_RETURN,
} StatementKind;

typedef union {
    ExprID ret_val;

    struct {
        char *ident;
        Type type;
    } declaration;

    struct {
        char *ident;
        ExprID expr;
    } assignment;
} StatementData;

typedef struct {
    StatementKind kind;
    StatementData data;
    StmtID next;
} Statement;

typedef struct NodePool_S *NodePool;

/**
 * @brief Create and initialize a NodePool
 *
 * @returns A valid instance of NodePool if sucessful, NULL otherwise
 */
NodePool pool_initialize();

/**
 * @brief Free the memory of the NodePool
 *
 * @note All IDs from the pool are considered invalid after calling this
 */
void pool_release(NodePool self);

/**
 * @brief Push a 'return' Statement into the pool
 *
 * @param[in] prev - ID of a previous statement or NO_ID
 * @param[in] expr - ID of a valid expression or NO_ID
 *
 * @returns The ID of the new node if successful, NO_ID otherwise
 */
StmtID pool_ret(NodePool self, StmtID prev, ExprID expr);

/**
 * @brief Push a 'declaration' Statement into the pool
 *
 * @param[in] prev - ID of a previous statement or NO_ID
 * @param[in] type - Datatype of the variable to be declared
 * @param[in] ident - Name of the variable to be declared
 *
 * @returns The ID of the new node if successful, NO_ID otherwise
 */
StmtID
pool_declaration(NodePool self, StmtID prev, Type type, const char *ident);

/**
 * @brief Push an 'assignment' Statement into the pool
 *
 * @param[in] prev - ID of a previous statement or NO_ID
 * @param[in] ident - Name of the variable to be declared
 * @param[in] expr - ID of a valid node from the pool
 *
 * @returns The ID of the new node if successful, NO_ID otherwise
 */
StmtID
pool_assignment(NodePool self, StmtID prev, const char *ident, ExprID expr);

/**
 * @brief Push an 'integer constant' Expression into the pool
 *
 * @param[in] constant - Value of the constant to push
 *
 * @returns The ID of the new node if successful, NO_ID otherwise
 */
ExprID pool_int_constant(NodePool self, int64_t constant);

/**
 * @brief Push a 'boolean constant' Statement into the pool
 *
 * @param[in] constant - Value of the constant to push
 *
 * @returns The ID of the new node if successful, NO_ID otherwise
 */
ExprID pool_bool_constant(NodePool self, bool constant);

/**
 * @brief Push a 'var' Statement into the pool
 *
 * @param[in] var - Name of the variable to push
 *
 * @returns The ID of the new node if successful, NO_ID otherwise
 */
ExprID pool_var(NodePool self, const char *var);

/**
 * @brief Push a 'binary' Statement into the pool
 *
 * @param[in] lhs - ID of a valid node from the pool
 * @param[in] rhs - ID of a valid node from the pool
 * @param[in] op - Binary operation between `lhs` and `rhs`
 *
 * @returns The ID of the new node if successful, NO_ID otherwise
 */
ExprID pool_binary(NodePool self, ExprID lhs, ExprID rhs, BinaryOp op);

/**
 * @brief Get a Statement from the pool
 *
 * @param[in] id - Valid ID of a Statement from the pool
 *
 * @returns A Statement if `id` represents a valid Statement, NULL otherwise
 */
const Statement *pool_get_stmt(const NodePool self, StmtID id);

/**
 * @brief Get an Expression from the pool
 *
 * @param[in] id - Valid ID of an Expression from the pool
 *
 * @returns An Expression if `id` represents a valid Expression, NULL otherwise
 */
const Expression *pool_get_expr(const NodePool self, ExprID id);

/**
 * @brief Display the subtree from the pool starting at `root`
 *
 * @param[in] root - ID of a valid Statement or Expression from the pool
 * @param[in] stream - Output handle to print the AST
 */
void pool_display(const NodePool self, NodeID root, FILE *stream);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _AST_H */
