#include <stdlib.h>
#include <string.h>

#include "hash_table.h"
#include "log.h"

#define HASH_TABLE_LOAD_THRESHOLD 0.75f

struct hash_table_bucket_element_st {
    char* key;
    void* value;
    struct hash_table_bucket_element_st* next;
};

struct hash_table_bucket_st {
    struct hash_table_bucket_element_st* first;
    struct hash_table_bucket_element_st* last;
};

struct hash_table_st {
    size_ty element_size;
    size_ty num_elements;
    size_ty num_hash_table_buckets;
    struct hash_table_bucket_st* hash_table_buckets;
};

static size_ty                              hash_string(const char* str);
static void                                 hash_table_bucket_append(struct hash_table_bucket_st* hash_table_bucket, struct hash_table_bucket_element_st* element);
static struct hash_table_bucket_element_st* hash_table_bucket_find_element(const struct hash_table_bucket_st* hash_table_bucket, const char* key);
static struct hash_table_bucket_st*         hash_table_find_bucket(const struct hash_table_st* hash_table, const char* key);
static void                                 hash_table_resize(struct hash_table_st* hash_table, size_ty num_buckets);

struct hash_table_st* hash_table_create(size_ty element_size) {
    struct hash_table_st* hash_table = malloc(sizeof(struct hash_table_st));
    hash_table->element_size = element_size;
    hash_table->num_elements = 0;
    hash_table->num_hash_table_buckets = 0;
    hash_table->hash_table_buckets = NULL;
    return hash_table;
}

void hash_table_destroy(struct hash_table_st** hash_table) {
    hash_table_clear(*hash_table);
    free(*hash_table);
    *hash_table = NULL;
}

size_ty hash_table_length(const struct hash_table_st* hash_table) {
    return hash_table->num_elements;
}

void* hash_table_get(const struct hash_table_st* hash_table, const char* key) {
    struct hash_table_bucket_element_st* bucket_element = hash_table_bucket_find_element(hash_table_find_bucket(hash_table, key), key);
    if (bucket_element == NULL)
        return NULL;
    return bucket_element->value;
}

void* hash_table_add(struct hash_table_st* hash_table, const char* key) {
    struct hash_table_bucket_st* bucket = NULL;
    struct hash_table_bucket_element_st* element = NULL;

    if (hash_table->num_hash_table_buckets == 0) {
        hash_table_resize(hash_table, 1);
        bucket = hash_table->hash_table_buckets;
    } else {
        bucket = hash_table_find_bucket(hash_table, key);
        element = hash_table_bucket_find_element(bucket, key);
        if (element != NULL)
            return element->value;

        if ((float)(hash_table->num_elements + 1) / hash_table->num_hash_table_buckets > HASH_TABLE_LOAD_THRESHOLD) {
            hash_table_resize(hash_table, hash_table->num_hash_table_buckets * 2);
            bucket = hash_table_find_bucket(hash_table, key);
        }
    }

    element = malloc(sizeof(struct hash_table_bucket_element_st));
    if (element == NULL) {
        log_error("Failed to allocate memory for hash_table bucket element.\n");
        return NULL;
    }

    element->key = strdup(key);
    element->value = malloc(hash_table->element_size);
    element->next = NULL;
    ++hash_table->num_elements;
    hash_table_bucket_append(bucket, element);

    return element->value;
}

void hash_table_clear(struct hash_table_st* hash_table) {
    for (size_ty i = 0; i < hash_table->num_hash_table_buckets; ++i) {
        struct hash_table_bucket_element_st* element = hash_table->hash_table_buckets[i].first;
        while (element != NULL) {
            struct hash_table_bucket_element_st* next = element->next;
            free(element->key);
            free(element->value);
            free(element);
            element = next;
        }
    }
    free(hash_table->hash_table_buckets);
    hash_table->num_elements = 0;
    hash_table->num_hash_table_buckets = 0;
    hash_table->hash_table_buckets = NULL;
}

void hash_table_iterate(const struct hash_table_st* hash_table, void(*function)(const char* key, void* value, void* user_ptr), void* user_ptr) {
    for (size_ty i = 0; i < hash_table->num_hash_table_buckets; ++i) {
        struct hash_table_bucket_element_st* element = hash_table->hash_table_buckets[i].first;
        while (element != NULL) {
            function(element->key, element->value, user_ptr);
            element = element->next;
        }
    }
}

size_ty hash_string(const char* str) {
    size_ty h = 0;
    for (unsigned char* p = (unsigned char*)str; *p != '\0'; p++)
        h = 37 * h + *p;
    return (size_ty)h;
}

void hash_table_bucket_append(struct hash_table_bucket_st* hash_table_bucket, struct hash_table_bucket_element_st* element) {
    if (hash_table_bucket->first == NULL) {
        hash_table_bucket->first = element;
        hash_table_bucket->last = hash_table_bucket->first;
    } else {
        hash_table_bucket->last->next = element;
        hash_table_bucket->last = hash_table_bucket->last->next;
    }
    hash_table_bucket->last->next = NULL;
}

struct hash_table_bucket_element_st* hash_table_bucket_find_element(const struct hash_table_bucket_st* hash_table_bucket, const char* key) {
    struct hash_table_bucket_element_st* bucket_element = hash_table_bucket->first;
    while (bucket_element != NULL) {
        if (strcmp(key, bucket_element->key) == 0)
            return bucket_element;
        bucket_element = bucket_element->next;
    }
    return NULL;
}

struct hash_table_bucket_st* hash_table_find_bucket(const struct hash_table_st* hash_table, const char* key) {
    return hash_table->hash_table_buckets + (hash_string(key) % hash_table->num_hash_table_buckets);
}

void hash_table_resize(struct hash_table_st* hash_table, size_ty num_buckets) {
    if (hash_table->num_hash_table_buckets == num_buckets)
        return;

    struct hash_table_bucket_st* old_buckets = hash_table->hash_table_buckets;
    const size_ty old_num_buckets = hash_table->num_hash_table_buckets;

    struct hash_table_bucket_st* new_hash_table_buckets = calloc(num_buckets, sizeof(struct hash_table_bucket_st)); 
    if (new_hash_table_buckets == NULL) {
        log_error("Failed to allocate memory for hash_table buckets.\n");
        return;
    }
    hash_table->num_hash_table_buckets = num_buckets;
    hash_table->hash_table_buckets = new_hash_table_buckets;

    for (size_ty i = 0; i < old_num_buckets; ++i) {
        struct hash_table_bucket_element_st* bucket_element = old_buckets[i].first;
        while (bucket_element != NULL) {
            struct hash_table_bucket_element_st* next = bucket_element->next;
            hash_table_bucket_append(hash_table_find_bucket(hash_table, bucket_element->key), bucket_element);
            bucket_element = next;
        }
    }

    free(old_buckets);
}

struct hash_tablei_bucket_element_st {
    size_ty key;
    void* value;
    struct hash_tablei_bucket_element_st* next;
};

struct hash_tablei_bucket_st {
    struct hash_tablei_bucket_element_st* first;
    struct hash_tablei_bucket_element_st* last;
};

struct hash_tablei_st {
    size_ty element_size;
    size_ty num_elements;
    size_ty num_hash_table_buckets;
    struct hash_tablei_bucket_st* hash_table_buckets;
};

static void                                  hash_tablei_bucket_append(struct hash_tablei_bucket_st* hash_table_bucket, struct hash_tablei_bucket_element_st* element);
static struct hash_tablei_bucket_element_st* hash_tablei_bucket_find_element(const struct hash_tablei_bucket_st* hash_table_bucket, size_ty key);
static struct hash_tablei_bucket_st*         hash_tablei_find_bucket(const struct hash_tablei_st* hash_table, size_ty key);
static void                                  hash_tablei_resize(struct hash_tablei_st* hash_table, size_ty num_buckets);

struct hash_tablei_st* hash_tablei_create(size_ty element_size) {
    struct hash_tablei_st* hash_table = malloc(sizeof(struct hash_tablei_st));
    hash_table->element_size = element_size;
    hash_table->num_elements = 0;
    hash_table->num_hash_table_buckets = 0;
    hash_table->hash_table_buckets = NULL;
    return hash_table;
}

void hash_tablei_destroy(struct hash_tablei_st** hash_table) {
    hash_tablei_clear(*hash_table);
    free(*hash_table);
    *hash_table = NULL;
}

size_ty hash_tablei_length(const struct hash_tablei_st* hash_table) {
    return hash_table->num_elements;
}

void* hash_tablei_get(const struct hash_tablei_st* hash_table, size_ty key) {
    struct hash_tablei_bucket_element_st* bucket_element = hash_tablei_bucket_find_element(hash_tablei_find_bucket(hash_table, key), key);
    if (bucket_element == NULL)
        return NULL;
    return bucket_element->value;
}

void* hash_tablei_add(struct hash_tablei_st* hash_table, size_ty key) {
    struct hash_tablei_bucket_st* bucket = NULL;
    struct hash_tablei_bucket_element_st* element = NULL;

    if (hash_table->num_hash_table_buckets == 0) {
        hash_tablei_resize(hash_table, 1);
        bucket = hash_table->hash_table_buckets;
    } else {
        bucket = hash_tablei_find_bucket(hash_table, key);
        element = hash_tablei_bucket_find_element(bucket, key);
        if (element != NULL)
            return element->value;

        if ((float)(hash_table->num_elements + 1) / hash_table->num_hash_table_buckets > HASH_TABLE_LOAD_THRESHOLD) {
            hash_tablei_resize(hash_table, hash_table->num_hash_table_buckets * 2);
            bucket = hash_tablei_find_bucket(hash_table, key);
        }
    }

    element = malloc(sizeof(struct hash_tablei_bucket_element_st));
    if (element == NULL) {
        log_error("Failed to allocate memory for hash_table bucket element.\n");
        return NULL;
    }

    element->key = key;
    element->value = malloc(hash_table->element_size);
    element->next = NULL;
    ++hash_table->num_elements;
    hash_tablei_bucket_append(bucket, element);

    return element->value;
}

void hash_tablei_clear(struct hash_tablei_st* hash_table) {
    for (size_ty i = 0; i < hash_table->num_hash_table_buckets; ++i) {
        struct hash_tablei_bucket_element_st* element = hash_table->hash_table_buckets[i].first;
        while (element != NULL) {
            struct hash_tablei_bucket_element_st* next = element->next;
            free(element->value);
            free(element);
            element = next;
        }
    }
    free(hash_table->hash_table_buckets);
    hash_table->num_elements = 0;
    hash_table->num_hash_table_buckets = 0;
    hash_table->hash_table_buckets = NULL;
}

void hash_tablei_iterate(const struct hash_tablei_st* hash_table, void(*function)(size_ty key, void* value, void* user_ptr), void* user_ptr) {
    for (size_ty i = 0; i < hash_table->num_hash_table_buckets; ++i) {
        struct hash_tablei_bucket_element_st* element = hash_table->hash_table_buckets[i].first;
        while (element != NULL) {
            function(element->key, element->value, user_ptr);
            element = element->next;
        }
    }
}

void hash_tablei_bucket_append(struct hash_tablei_bucket_st* hash_table_bucket, struct hash_tablei_bucket_element_st* element) {
    if (hash_table_bucket->first == NULL) {
        hash_table_bucket->first = element;
        hash_table_bucket->last = hash_table_bucket->first;
    } else {
        hash_table_bucket->last->next = element;
        hash_table_bucket->last = hash_table_bucket->last->next;
    }
    hash_table_bucket->last->next = NULL;
}

struct hash_tablei_bucket_element_st* hash_tablei_bucket_find_element(const struct hash_tablei_bucket_st* hash_table_bucket, size_ty key) {
    struct hash_tablei_bucket_element_st* bucket_element = hash_table_bucket->first;
    while (bucket_element != NULL) {
        if (key == bucket_element->key)
            return bucket_element;
        bucket_element = bucket_element->next;
    }
    return NULL;
}

struct hash_tablei_bucket_st* hash_tablei_find_bucket(const struct hash_tablei_st* hash_table, size_ty key) {
    return hash_table->hash_table_buckets + (key % hash_table->num_hash_table_buckets);
}

void hash_tablei_resize(struct hash_tablei_st* hash_table, size_ty num_buckets) {
    if (hash_table->num_hash_table_buckets == num_buckets)
        return;

    struct hash_tablei_bucket_st* old_buckets = hash_table->hash_table_buckets;
    const size_ty old_num_buckets = hash_table->num_hash_table_buckets;

    struct hash_tablei_bucket_st* new_hash_table_buckets = calloc(num_buckets, sizeof(struct hash_tablei_bucket_st)); 
    if (new_hash_table_buckets == NULL) {
        log_error("Failed to allocate memory for hash_table buckets.\n");
        return;
    }
    hash_table->num_hash_table_buckets = num_buckets;
    hash_table->hash_table_buckets = new_hash_table_buckets;

    for (size_ty i = 0; i < old_num_buckets; ++i) {
        struct hash_tablei_bucket_element_st* bucket_element = old_buckets[i].first;
        while (bucket_element != NULL) {
            struct hash_tablei_bucket_element_st* next = bucket_element->next;
            hash_tablei_bucket_append(hash_tablei_find_bucket(hash_table, bucket_element->key), bucket_element);
            bucket_element = next;
        }
    }

    free(old_buckets);
}