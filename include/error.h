#ifndef _ERROR_H
#define _ERROR_H

typedef enum {
    Status_OK,
    Status_TypeError,
    Status_MultipleDeclaration,
    Status_UndeclSymbol,
    Status_UninitSymbol,
} Status;

#endif /* _ERROR_H */
