#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "str_pool.h"

struct StrPool_S {
    char *data;
    size_t size;
    size_t capacity;
};

StrPool str_pool_init() {
    StrPool self = (StrPool)calloc(1, sizeof(struct StrPool_S));
    return self;
}

void str_pool_release(StrPool self) {
    if (self == NULL) {
        return;
    }

    free(self->data);
    free(self);
}

StrID str_pool_put(StrPool self, const char *sym) {
    // already in table
    for (StrID i = 0; i < self->size; i += strlen(&self->data[i]) + 1) {
        if (strcmp(sym, &self->data[i]) == 0) {
            return i;
        }
    }
    // must add to table
    const size_t new_size = self->size + strlen(sym) + 1;
    if (new_size >= self->capacity) {
        size_t new_capacity = new_size;
        // compute the next highest power of 2 of 64-bit new_capacity
        new_capacity--;
        new_capacity |= new_capacity >> 1;
        new_capacity |= new_capacity >> 2;
        new_capacity |= new_capacity >> 4;
        new_capacity |= new_capacity >> 8;
        new_capacity |= new_capacity >> 16;
        new_capacity |= new_capacity >> 32;
        new_capacity++;

        // char *new_data = realloc(self->data, new_capacity);
        self->data = realloc(self->data, new_capacity);
        if (self->data == NULL) {
            return NO_ID;
        }
        // self->data = new_data;
        self->capacity = new_capacity;
    }

    const StrID res = self->size;
    self->size = new_size;

    strcpy(&self->data[res], sym);

    return res;
}

const char *str_pool_get(StrPool self, StrID id) {
    if (self->size <= id) {
        return NULL;
    }

    return &self->data[id];
}

