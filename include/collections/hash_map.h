#ifndef OASIS_C_COLLECTIONS_HASH_MAP_H
#define OASIS_C_COLLECTIONS_HASH_MAP_H

#include <stddef.h>
#include <stdlib.h>

#define DEFAULT_CAPACITY 32;

// ==== interface ====

typedef struct hash_map_entry hash_map_entry;
typedef struct hash_map hash_map;

// lifecycle

hash_map* hm_create(uint64_t (*hash_fn)(void*), double load_factor);
void hm_destroy(hash_map* hm);

// crud

bool hm_is_empty(hash_map* hm);
hash_map_entry* hm_get(hash_map* hm, void* key);
bool hm_contains(hash_map* hm, void* key);
void hm_insert(hash_map* hm, void* key, void* value);
void hm_remove(hash_map* hm, void* key);

// internal APIs

hash_map_entry* __hm_find_slot(hash_map* hm, void* key, bool allow_empty);

// ==== implementation ====

struct hash_map_entry {
  uint64_t hash;
  void* key;
  void* value;
};

struct hash_map {
  uint64_t (*hash_fn)(void*);
  double load_factor; 
  hash_map_entry* entries;
  size_t length;
  size_t capacity;
};

hash_map* hm_create(uint64_t (*hash_fn)(void*), double load_factor) {
  hash_map* hm = (hash_map*)malloc(sizeof(hash_map));
  if (hm == NULL) {
    return NULL;
  }
  hm->hash_fn = hash_fn;
  hm->load_factor = load_factor;
  hm->length = 0;
  hm->capacity = DEFAULT_CAPACITY;
  hm->entries = (hash_map_entry*)calloc(hm->capacity, sizeof(hash_map_entry));
  if (hm->entries == NULL) {
    free(hm);
    return NULL;
  }
  return hm;
}

void hm_destroy(hash_map* hm) {
  free(hm->entries);
}

bool hm_is_empty(hash_map* hm) {
  return hm->length == 0;
}

hash_map_entry* hm_get(hash_map* hm, void* key) {
  return __hm_find_slot(hm, key, false);
}

bool hm_contains(hash_map* hm, void* key) {
  return hm_get(hm, key) != NULL;
}

void hm_insert(hash_map* hm, void* key, void* value) {
  hash_map_entry* entry = __hm_find_slot(hm, key, true);

  if (entry == NULL) {
    assert(false);
  }

  uint64_t hash = hm->hash_fn(key);
  if (entry->key == NULL) {
    hm->length += 1;
  }
  entry->hash = hash;
  entry->key = key;
  entry->value = value;
}

void hm_remove(hash_map* hm, void* key) {
  hash_map_entry* entry = __hm_find_slot(hm, key, false);

  if (entry == NULL) {
    return;
  }

  assert(entry->key != NULL);
  hm->length -= 1;
  entry->hash = 0;
  entry->key = NULL;
  entry->value = NULL;
}

hash_map_entry* __hm_find_slot(hash_map* hm, void* key, bool allow_empty) {
  uint64_t hash = hm->hash_fn(key);
  size_t original_slot = hash % hm->capacity;
  size_t current_slot = original_slot;
  while (true) {
    hash_map_entry* entry = &hm->entries[current_slot];
    if (entry->key == NULL) {
      if (allow_empty) {
        return entry;
      }

      return NULL;
    } else {
      // if the entry is our hash, return the entry
      if (entry->hash == hash) {
        return entry;
      }

      // otherwise we need to linearly probe through the rest of the list to find
      // either the next empty spot (element not found) or the correct entry (hash matches)
      current_slot++;

      // if we've overflowed our capacity, roll around to the front 
      if (current_slot > hm->capacity) {
        current_slot = 0;
      }

      // if we've made it back to the original slot, this shouldn't be possible
      // because it means we're not following our load factor but we're not going
      // to crash the process so just return null
      if (current_slot == original_slot) {
        return NULL;
      }
    }
  }
}

#endif // OASIS_C_COLLECTIONS_HASH_MAP_H
