#ifndef _STR_POOL
#define _STR_POOL

#include <stdint.h>

#define NO_ID UINT32_MAX

typedef uint32_t StrID;

typedef struct StrPool_S *StrPool;

StrPool str_pool_init();
void str_pool_release(StrPool self);

StrID str_pool_put(StrPool self, const char *sym);
const char *str_pool_get(StrPool self, StrID id);

#endif
