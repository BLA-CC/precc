#ifndef _INTERP_H
#define _INTERP_H

#include "ast.h"
#include "defs.h"
#include "str_pool.h"
#include "sym_table.h"

Sym interp(Ast ast, NodeID root, StrPool strs, SymTable syms);

#endif /* _INTERP_H */
