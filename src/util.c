#include "util.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

char *u_strdup(const char *src) {
    size_t n = strlen(src);
    char *dest = (char *)calloc(n + 1, sizeof(*dest));

    if (dest == NULL) {
        return NULL;
    }

    memcpy(dest, src, n);

    return dest;
}
