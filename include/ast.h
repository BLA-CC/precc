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
    StrID var;
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
        StrID ident;
        Type type;
    } declaration;

    struct {
        StrID ident;
        ExprID expr;
    } assignment;
} StatementData;

typedef struct {
    StatementKind kind;
    StatementData data;
    StmtID next;
} Statement;

typedef struct Pool_S *AST;

/**
 * @brief Create and initialize the AST
 *
 * @returns A valid instance of AST if sucessful, NULL otherwise
 */
AST ast_initialize();

/**
 * @brief Free the memory of the AST
 *
 * @note All IDs from the AST are considered invalid after calling this
 */
void ast_release(AST self);

/**
 * @brief Push a 'return' Statement into the AST
 *
 * @param[in] prev - ID of a previous statement or NO_ID
 * @param[in] expr - ID of a valid expression or NO_ID
 *
 * @returns The ID of the new node if successful, NO_ID otherwise
 */
StmtID ast_ret(AST self, StmtID prev, ExprID expr);

/**
 * @brief Push a 'declaration' Statement into the AST
 *
 * @param[in] prev - ID of a previous statement or NO_ID
 * @param[in] type - Datatype of the variable to be declared
 * @param[in] ident - Name of the variable to be declared
 *
 * @returns The ID of the new node if successful, NO_ID otherwise
 */
StmtID
ast_declaration(AST self, StmtID prev, Type type, StrID ident);

/**
 * @brief Push an 'assignment' Statement into the AST
 *
 * @param[in] prev - ID of a previous statement or NO_ID
 * @param[in] ident - Name of the variable to be declared
 * @param[in] expr - ID of a valid node from the AST
 *
 * @returns The ID of the new node if successful, NO_ID otherwise
 */
StmtID
ast_assignment(AST self, StmtID prev, StrID ident, ExprID expr);

/**
 * @brief Push an 'integer constant' Expression into the AST
 *
 * @param[in] constant - Value of the constant to push
 *
 * @returns The ID of the new node if successful, NO_ID otherwise
 */
ExprID ast_int_constant(AST self, int64_t constant);

/**
 * @brief Push a 'boolean constant' Statement into the AST
 *
 * @param[in] constant - Value of the constant to push
 *
 * @returns The ID of the new node if successful, NO_ID otherwise
 */
ExprID ast_bool_constant(AST self, bool constant);

/**
 * @brief Push a 'var' Statement into the AST
 *
 * @param[in] var - Name of the variable to push
 *
 * @returns The ID of the new node if successful, NO_ID otherwise
 */
ExprID ast_var(AST self, StrID var);

/**
 * @brief Push a 'binary' Statement into the AST
 *
 * @param[in] lhs - ID of a valid node from the AST
 * @param[in] rhs - ID of a valid node from the AST
 * @param[in] op - Binary operation between `lhs` and `rhs`
 *
 * @returns The ID of the new node if successful, NO_ID otherwise
 */
ExprID ast_binary(AST self, ExprID lhs, ExprID rhs, BinaryOp op);

/**
 * @brief Get a Statement from the AST
 *
 * @param[in] id - Valid ID of a Statement from the AST
 *
 * @returns A Statement if `id` represents a valid Statement, NULL otherwise
 */
const Statement *ast_get_stmt(const AST self, StmtID id);

/**
 * @brief Get an Expression from the AST
 *
 * @param[in] id - Valid ID of an Expression from the AST
 *
 * @returns An Expression if `id` represents a valid Expression, NULL otherwise
 */
const Expression *ast_get_expr(const AST self, ExprID id);

/**
 * @brief Display the subtree from the AST starting at `root`
 *
 * @param[in] root - ID of a valid Statement or Expression from the AST
 * @param[in] stream - Output handle to print the AST
 */
void ast_display(const AST self, NodeID root, StrPool strs, FILE *stream);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _AST_H */
