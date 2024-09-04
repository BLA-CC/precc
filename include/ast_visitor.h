#ifndef AST_VISITOR_H
#define AST_VISITOR_H

#include "ast.h"
#include "str_pool.h"

// FIXME: redefine this better
typedef int Status; 

typedef struct Visitor_S *Visitor;

/**
 * @brief Initializes the Visitor with the given callbacks and data.
 *
 * @param[in] ast The abstract syntax tree to traverse.
 * @param[in] strs String pool for managing strings in the AST.
 * @param[in] additional_args Any additional arguments required by the visitor.
 * @param[in] visit_int_constant Callback for visiting integer constant expressions.
 * @param[in] visit_bool_constant Callback for visiting boolean constant expressions.
 * @param[in] visit_var Callback for visiting variable expressions.
 * @param[in] visit_binary_expr Callback for visiting binary expressions.
 * @param[in] visit_declaration Callback for visiting declaration statements.
 * @param[in] visit_assignment Callback for visiting assignment statements.
 * @param[in] visit_return Callback for visiting return statements.
 *
 * @returns Initialized visitor object with the provided callbacks.
 */
Visitor init_visitor(
    AST ast,
    StrPool strs,
    void *additional_args,
    Status (*visit_int_constant)(Visitor visitor, Expression *expr),
    Status (*visit_bool_constant)(Visitor visitor, Expression *expr),
    Status (*visit_var)(Visitor visitor, Expression *expr),
    Status (*visit_binary_expr)(Visitor visitor, Expression *expr),
    Status (*visit_declaration)(Visitor visitor, Statement *stmt),
    Status (*visit_assignment)(Visitor visitor, Statement *stmt),
    Status (*visit_return)(Visitor visitor, Statement *stmt)
);

/**
 * @brief Traverses the AST from the given root node, visiting expressions 
 * and statements based on the visitor callbacks.
 *
 * @param[in] self The visitor responsible for traversing the AST.
 * @param[in] node_id The ID of the root node in the AST to begin traversal.
 */


void ast_visit(Visitor self, NodeID node_id);

/**
 * @brief Visits a specific expression node within the AST.
 *
 * This function can be used inside the callback functions to continue
 * traversing the AST.
 *
 * @param[in] self The visitor performing the traversal.
 * @param[in] expr The expression node to visit.
 */
void visit_expr(Visitor self, Expression *expr);

/**
 * @brief Visits a specific statement node within the AST.
 *
 * This function can be used inside the callback functions to continue
 * traversing the AST.
 *
 * @param[in] self The visitor performing the traversal.
 * @param[in] stmt The statement node to visit.
 */


void visit_stmt(Visitor self, Statement *stmt);

/**
 * @brief Releases the memory associated with a visitor.
 *
 * @param[in] self - A Visitor
 */
void visitor_release(Visitor self);


//
// USE CASE EXAMPLE OF A VISITOR:
//

/**
 * @brief Displays the subtree of the AST starting from the given root node.
 *
 * @param[in] node_id The ID of a valid Statement or Expression in the AST.
 * @param[in] stream The output handle where the AST subtree will be printed.
 */

void ast_display(const AST ast, NodeID node_id, StrPool strs, FILE *stream);

#endif // AST_VISITOR_H
