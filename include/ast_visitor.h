#ifndef AST_VISITOR_H
#define AST_VISITOR_H

#include "ast.h"
#include "str_pool.h"
#include "error.h"

typedef struct Visitor_S *Visitor;

/**
 * @brief Initializes a visitor to traverse an Abstract Syntax Tree (AST).
 *
 * This function sets up a Visitor structure with user-defined callback
 * functions for handling various expression and statement types.
 *
 * @param[in] ast              The Abstract Syntax Tree to traverse.
 * @param[in] strs             String pool for managing identifier strings.
 * @param[in] additional_args  User-provided arguments for customized traversal.
 * @param[in] visit_<node>     Callback function for visiting <node>
 * @return A newly initialized Visitor, or NULL if initialization fails.
 */
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
    Status (*visit_main)(Visitor visitor, NodeID stmt_id));

/**
 * @brief Traverses the AST from the given root node, visiting expressions
 * and statements based on the visitor callbacks. It will print errors in stderr
 * if there is any.
 *
 * @param[in] self The visitor responsible for traversing the AST.
 * @param[in] node_id The ID of the root node in the AST to begin traversal.
 *
 * @return Status of the traversal operation.
 */
Status ast_visit(Visitor self, NodeID node_id);

/**
 * @brief Visits a specific expression node within the AST.
 *
 * This function can be used inside the callback functions to continue
 * traversing the AST. It will print errors in stderr if there is any.
 *
 * @param[in] self The visitor performing the traversal.
 * @param[in] expr The ID of the expression node to visit.
 *
 * @return Status of the visit operation.
 */
Status visit_expr(Visitor self, NodeID expr_id);

/**
 * @brief Visits a specific statement node within the AST.
 *
 * This function can be used inside the callback functions to continue
 * traversing the AST. It will print errors in stderr if there is any.
 *
 * @param[in] self The visitor performing the traversal.
 * @param[in] stmt_id The ID of the statement node to visit.
 *
 * @return Status of the visit operation.
 */
Status visit_stmt(Visitor self, NodeID stmt_id);

/**
 * @brief Interrupt the flow of the visitor, effectively stopping
 * traversal of the AST.
 *
 * @param[in] self The visitor performing the traversal.
 */
void visitor_interrupt(Visitor self);

/**
 * @brief Releases the memory associated with a visitor.
 *
 * This function frees all memory allocated by the visitor, including any
 * internal data structures or additional arguments.
 *
 * @param[in] self The visitor to release.
 */
void visitor_release(Visitor self);

/**
 * @brief Retrieves the additional arguments passed during visitor
 * initialization.
 *
 * @param[in] self The visitor structure.
 *
 * @return Pointer to the user-provided additional arguments.
 */
void *visitor_get_context(Visitor self);

/**
 * @brief Retrieves the AST associated with the visitor.
 *
 * @param[in] self The visitor structure.
 *
 * @return The AST being traversed.
 */
Ast visitor_get_ast(Visitor self);

/**
 * @brief Retrieves the string pool associated with the visitor.
 *
 * @param[in] self The visitor structure.
 *
 * @return The string pool used for managing identifiers.
 */
StrPool visitor_get_strs(Visitor self);

//
// USE CASE EXAMPLE OF A VISITOR:
//

/**
 * @brief Displays the subtree of the AST starting from the given root node.
 *
 * @param[in] node_id The ID of a valid statement or expression in the AST.
 * @param[in] stream The output handle where the AST subtree will be printed.
 */
void ast_display(const Ast ast, NodeID node_id, StrPool strs, FILE *stream);

#endif // AST_VISITOR_H
