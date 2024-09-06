#ifndef _ERROR_H
#define _ERROR_H

typedef enum {
    Status_OK,

    // expected type T but got a different type
    Status_TypeError,

    // use of uninitialized symbol
    Status_UninitSymbol,

    // use of undeclared symbol
    Status_UndeclSymbol,

    // re-declaration of an already declared symbol
    Status_MultiDeclSymbol,
} Status;

#endif /* _ERROR_H */
