#ifndef _SYMTABLE_H
#define _SYMTABLE_H

#include <stdbool.h>
#include <stdio.h>

#include "defs.h"
#include "ast.h"
#include "str_pool.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef union {
    bool v_bool;
    int64_t v_int;
} SymValue;

typedef struct {
    StrID ident;
    Type type;
    SymValue value;
} Sym;

typedef struct SymNode_S *SymNode;
typedef struct SymTable_S *SymTable;

/**
 * @brief Initializes a new symbol table.
 *
 * @returns A pointer to the new symbol table if successful, or NULL if the
 * initialization fails.
 */
SymTable symtable_initialize();

/**
 * @brief Retrieves symbol information from the symbol table.
 *
 * @param[in] self  A pointer to the symbol table.
 * @param[in] ident The identifier of the symbol to retrieve.
 *
 * @returns A pointer to the symbol node containing information about the
 * symbol, or NULL if the symbol is not found.
 */
SymNode symtable_get_info(const SymTable self, StrID ident);

/**
 * @brief Adds a new symbol to the symbol table.
 *
 * @param[in] self  A pointer to the symbol table.
 * @param[in] ident The identifier of the symbol to add.
 * @param[in] type  The type of the symbol to add.
 */
void symtable_add_symbol(SymTable self, const StrID ident, const Type type);

/**
 * @brief Releases the memory associated with a symbol table.
 *
 * @param[in] self A pointer to the symbol table to be released.
 */
void symtable_release(SymTable self);

/**
 * @brief Retrieves the symbol data from a node.
 *
 * @param[in] symnode A pointer to the symbol node from which to retrieve the
 * symbol data.
 *
 * @returns The symbol structure containing information about the symbol.
 */
Sym *symnode_get_symbol(SymNode symnode);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _SYMTABLE_H */
