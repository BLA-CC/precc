#ifndef SEMPASS_H
#define SEMPASS_H

#include "../include/ast_visitor.h"
#include "../include/sym_table.h"
#include "../include/error.h"

/**
 * @brief Perform semantic analysis on the given AST node.
 *
 * This function traverses the AST from the specified node and performs type
 * checking, declaration validation, and other semantic checks. It uses a
 * visitor pattern to walk through the AST and ensure the correctness of the
 * program's semantics.
 *
 * @param[in] ast - The abstract syntax tree (AST) to be analyzed
 * @param[in] node_id - The starting node ID for the analysis
 * @param[in] strs - String pool for handling string identifiers
 *
 * @returns Status_OK if the semantic analysis is successful, or an appropriate
 * error status otherwise.
 */
Status sempass(const Ast ast, NodeID node_id, StrPool strs);

#endif // SEMPASS_H
