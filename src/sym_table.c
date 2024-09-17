#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sym_table.h"
#include "ast.h"

#define DEFAULT_SIZE 64;

//////// SYMBOL //////////////

struct SymNode_S {
    bool has_value;
    Sym symbol;
    SymNode _next;
};

Sym *symnode_get_symbol(SymNode symnode) {
    return &symnode->symbol;
}

SymNode _create_symbol(const StrID ident, const Type type) {
    SymNode self = (SymNode)malloc(sizeof(struct SymNode_S));

    self->has_value = false;
    self->symbol.ident = ident;
    self->symbol.type = type;
    self->_next = NULL;

    return self;
};

void _set_symbol(SymNode self, const SymValue value) {
    self->has_value = true;
    self->symbol.value = value;
}

/////////// SYMBOL TABLE ////////////////

struct SymTable_S {
    size_t size;
    SymNode *hash_table; // Separate Chaining algorithm
};

// symbol table constructor
SymTable symtable_initialize() {
    SymTable self = (SymTable)malloc(sizeof(struct SymTable_S));

    if (self == NULL)
        return NULL;

    self->size = DEFAULT_SIZE;
    self->hash_table = (SymNode *)calloc(self->size, sizeof(SymNode));

    if (self->hash_table == NULL) {
        free(self);
        return NULL;
    }

    return self;
}

int _hash_function(const SymTable self, const StrID key) {
    return key % self->size;
}

void _symtable_insert(SymTable self, SymNode symnode) {
    int bucket_idx = _hash_function(self, symnode->symbol.ident);

    if (self->hash_table[bucket_idx] == NULL) {
        self->hash_table[bucket_idx] = symnode;
    } else {
        symnode->_next = self->hash_table[bucket_idx];
        self->hash_table[bucket_idx] = symnode;
    }
}

void symtable_release(SymTable self) {
    SymNode next_node, current_node;

    for (size_t i = 0; i < self->size; i++) {
        current_node = self->hash_table[i];

        while (current_node != NULL) {
            next_node = current_node->_next;
            free(current_node);
            current_node = next_node;
        }
    }
    free(self->hash_table);
    free(self);
}

SymNode symtable_get_info(const SymTable self, StrID ident) {
    int bucket_idx = _hash_function(self, ident);
    SymNode sym_node = self->hash_table[bucket_idx];

    while (sym_node != NULL) {
        if (sym_node->symbol.ident == ident) {
            return sym_node;
        }
        sym_node = sym_node->_next;
    }
    return NULL;
}

void symtable_add_symbol(SymTable self, const StrID ident, const Type type) {
    SymNode symnode = _create_symbol(ident, type);
    _symtable_insert(self, symnode);
}
