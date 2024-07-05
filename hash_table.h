#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "common_types.h"

struct hash_table_st;
struct hash_tablei_st;

struct hash_table_st* hash_table_create(size_ty element_size);
void                  hash_table_destroy(struct hash_table_st** hash_table);
size_ty               hash_table_length(const struct hash_table_st* hash_table);
void*                 hash_table_get(const struct hash_table_st* hash_table, const char* key);
void*                 hash_table_add(struct hash_table_st* hash_table, const char* key);
void                  hash_table_clear(struct hash_table_st* hash_table);
void                  hash_table_iterate(const struct hash_table_st* hash_table, void(*function)(const char* key, void* value, void* user_ptr), void* user_ptr);

struct hash_tablei_st* hash_tablei_create(size_ty element_size);
void                   hash_tablei_destroy(struct hash_tablei_st** hash_table);
size_ty                hash_tablei_length(const struct hash_tablei_st* hash_table);
void*                  hash_tablei_get(const struct hash_tablei_st* hash_table, size_ty key);
void*                  hash_tablei_add(struct hash_tablei_st* hash_table, size_ty key);
void                   hash_tablei_clear(struct hash_tablei_st* hash_table);
void                   hash_tablei_iterate(const struct hash_tablei_st* hash_table, void(*function)(size_ty key, void* value, void* user_ptr), void* user_ptr);

#endif