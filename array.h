#ifndef ARRAY_H
#define ARRAY_H

#include "common_types.h"

struct array_st;

struct array_st* array_create(size_ty element_size);
void             array_destroy(struct array_st** array);
size_ty          array_length(const struct array_st* array);
void*            array_get(const struct array_st* array, size_ty index);
void*            array_push(struct array_st* array);
void             array_clear(struct array_st* array);
void             array_set_capacity(struct array_st* array, size_ty capacity);
void             array_shrink(struct array_st* array);
void             array_sort(struct array_st* array, bool_ty (*compare)(const void* a, const void* b));

#endif