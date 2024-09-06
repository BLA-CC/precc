#ifndef AST_VISITOR_H
#define AST_VISITOR_H

#include "ast.h"
#include "str_pool.h"

// FIXME: redefine this better
typedef int Status; 

typedef struct Visitor_S *Visitor;

// TODO: docs
Visitor init_visitor(
    AST ast,
    StrPool strs,
    void *additional_args,
    Status (*visit_int_constant)(Visitor visitor, ExprID expr_id),
    Status (*visit_bool_constant)(Visitor visitor, ExprID expr_id),
    Status (*visit_var)(Visitor visitor, ExprID expr_id),
    Status (*visit_binary_expr)(Visitor visitor, ExprID expr_id),
    Status (*visit_declaration)(Visitor visitor, StmtID stmt_id),
    Status (*visit_assignment)(Visitor visitor, StmtID stmt_id),
    Status (*visit_return)(Visitor visitor, StmtID stmt_id)
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
 * @param[in] expr The ID of the expression node to visit.
 */
void visit_expr(Visitor self, ExprID expr_id);

/**
 * @brief Visits a specific statement node within the AST.
 *
 * This function can be used inside the callback functions to continue
 * traversing the AST.
 *
 * @param[in] self The visitor performing the traversal.
 * @param[in] stmt_id The ID of the statement node to visit.
 */


void visit_stmt(Visitor self, StmtID stmt_id);

/**
 * @brief Releases the memory associated with a visitor.
 *
 * @param[in] self - A Visitor
 */
void visitor_release(Visitor self);

// TODO: document
void *visitor_get_additional_args(Visitor self);
AST visitor_get_ast(Visitor self);

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
