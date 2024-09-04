#ifndef _RESULT_H
#define _RESULT_H

#define RESULT(ok_t, err_t)                                                    \
    struct {                                                                   \
        union {                                                                \
            ok_t ok;                                                           \
            err_t err;                                                         \
        };                                                                     \
        ResultKind res;                                                        \
    }

#define IS_OK(__r)  (__r.res == ResultKind_OK)
#define IS_ERR(__r) (__r.res == ResultKind_ERR)

typedef enum {
    ResultKind_ERR,
    ResultKind_OK,
} ResultKind;

#endif /* _RESULT_H */
