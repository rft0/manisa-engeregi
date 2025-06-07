#ifndef __HASHMAP_H
#define __HASHMAP_H

#include <stdlib.h>
#include <stdint.h>

#define hashmap_lit_str(str) (str), (sizeof(str) - 1)

typedef struct _HashMap HashMap;

HashMap* hashmap_new();

int hashmap_get(HashMap* map, const void* key, size_t key_len, uintptr_t* out);
int hashmap_set(HashMap* map, const void* key, size_t key_len, uintptr_t value);
size_t hashmap_size(const HashMap* map);

typedef void (*hashmap_iterate_fn)(const void* key, size_t key_len, uintptr_t value, void* user_data);
void hashmap_iterate(HashMap* map, hashmap_iterate_fn fn, void* user_data);

void hashmap_free(HashMap* map);


#endif