#include <stdlib.h>
#include <string.h>

#include "array.h"

struct array_st {
    size_ty element_size;
    size_ty capacity;
    size_ty length;
    void* elements;
};

void array_quick_sort(struct array_st* array, bool_ty (*compare)(const void* a, const void* b));
void array_quick_sort_partition(struct array_st* array, int start, int end, bool_ty (*compare)(const void* a, const void* b));

struct array_st* array_create(size_ty element_size) {
    struct array_st* array = malloc(sizeof(struct array_st));
    array->element_size = element_size;
    array->capacity = 0;
    array->length = 0;
    array->elements = NULL;
    return array;
}

void array_destroy(struct array_st** array) {
    free((*array)->elements);
    free(*array);
    *array = NULL;
}

size_ty array_length(const struct array_st* array) {
    return array->length;
}

void* array_get(const struct array_st* array, size_ty index) {
    if (index >= array->length)
        return NULL;
    return (void*)(((char*)array->elements) + (index * array->element_size));
}

void* array_push(struct array_st* array) {
    if (array->length == array->capacity)
        array_set_capacity(array, array->capacity == 0 ? 1 : array->capacity * 2);
    return array_get(array, array->length++);
}

void array_clear(struct array_st* array) {
    array->length = 0;
    array->capacity = 0;
    free(array->elements);
    array->elements = NULL;
}

void array_set_capacity(struct array_st* array, size_ty capacity) {
    if (array->capacity == capacity)
        return;
    array->capacity = capacity;
    array->elements = realloc(array->elements, (size_t)(array->capacity * array->element_size));
    if (array->length > array->capacity)
        array->length = array->capacity;
}

void array_shrink(struct array_st* array) {
    array_set_capacity(array, array->length);
}

void array_sort(struct array_st* array, bool_ty (*compare)(const void* a, const void* b)) {
    array_quick_sort(array, compare);
}

void array_quick_sort(struct array_st* array, bool_ty (*compare)(const void* a, const void* b)) {
    array_quick_sort_partition(array, (int)0, (int)array->length - 1, compare);
}

void array_quick_sort_partition(struct array_st* array, int start, int end, bool_ty (*compare)(const void* a, const void* b)) {
    if (end <= start)
        return;

    int i = start - 1;
    int j = start;
    void* element_pivot = (char*)array->elements + end * array->element_size;
    void* element_temp = malloc(array->element_size);
    void* element_i = NULL;
    void* element_j = NULL;

    while (j != end) {
        element_j = (char*)array->elements + j * array->element_size;
        if (compare(element_j, element_pivot) && ++i != j) {
            element_i = (char*)array->elements + i * array->element_size;
            memcpy(element_temp, element_i, array->element_size);
            memcpy(element_i, element_j, array->element_size);
            memcpy(element_j, element_temp, array->element_size);
        }
        ++j;
    }

    element_i = (char*)array->elements + ++i * array->element_size;
    memcpy(element_temp, element_i, array->element_size);
    memcpy(element_i, element_pivot, array->element_size);
    memcpy(element_pivot, element_temp, array->element_size);
    free(element_temp);

    array_quick_sort_partition(array, start, i - 1, compare);
    array_quick_sort_partition(array, i + 1, end, compare);
}