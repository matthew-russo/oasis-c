#ifndef OASIS_C_COLLECTIONS_HASH_MAP_H
#define OASIS_C_COLLECTIONS_HASH_MAP_H

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#define HM_DEFAULT_CAPACITY 32

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

hash_map_entry* __hm_find_slot(hash_map* hm, void* key, bool allow_empty, size_t* slot);
void __hm_grow(hash_map* hm);

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
  hm->capacity = HM_DEFAULT_CAPACITY;
  hm->entries = (hash_map_entry*)calloc(hm->capacity, sizeof(hash_map_entry));
  if (hm->entries == NULL) {
    free(hm);
    return NULL;
  }
  return hm;
}

void hm_destroy(hash_map* hm) {
  free(hm->entries);
  hm->hash_fn = NULL;
  hm->length = 0;
  hm->capacity = 0;
  free(hm);
}

bool hm_is_empty(hash_map* hm) {
  return hm->length == 0;
}

hash_map_entry* hm_get(hash_map* hm, void* key) {
  return __hm_find_slot(hm, key, false, NULL);
}

bool hm_contains(hash_map* hm, void* key) {
  return hm_get(hm, key) != NULL;
}

void hm_insert(hash_map* hm, void* key, void* value) {
  // determine if we need to resize by checking if our current size + 1
  // will exceed the load factor
  if ((double)(hm->length + 1) / (double)hm->capacity > hm->load_factor) {
    __hm_grow(hm);
  }

  size_t slot;
  hash_map_entry* entry = NULL;
  int grows = 0;
  while (entry == NULL) {
    entry = __hm_find_slot(hm, key, true, &slot);
    if (entry == NULL) {
      __hm_grow(hm);
      grows++;
    }

    if (grows == 3) {
      assert(false);
    }
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
  size_t original_slot;
  hash_map_entry* entry = __hm_find_slot(hm, key, false, &original_slot);

  if (entry == NULL) {
    return;
  }

  // remove the key, which consists of zero'ing our entry and decrementing the length
  assert(entry->key != NULL);
  hm->length -= 1;
  entry->hash = 0;
  entry->key = NULL;
  entry->value = NULL;

  // now that we've removed the key, we need to maintain any potential chains. to
  // do this, we need to scan forward through the slots (wrapping back to 0 if
  // needed) and if an entry is present, and its key's hash does *not* match the
  // slot its in, we shift if left by one
  size_t current_slot = original_slot;
  while (true) {
    size_t next_slot = current_slot + 1;
    if (next_slot >= hm->capacity) {
      next_slot = 0;
    }

    // if we've wrapped all the way back to the original slot we've started at, exit
    if (next_slot == original_slot) {
      return;
    }

    // if there is no next item, exit
    hash_map_entry next = hm->entries[next_slot];
    if (next.key == NULL) {
      assert(next.hash == 0);
      assert(next.value == NULL);
      return;
    }

    uint64_t next_hash = hm->hash_fn(next.key);
    size_t intended_next_slot = next_hash % hm->capacity;

    // if the next item is in the right spot, exit
    if (next_slot == intended_next_slot) {
      return;
    }

    // otherwise start shifting items left
    hm->entries[current_slot] = next;
    hm->entries[next_slot].hash = 0;
    hm->entries[next_slot].key = NULL;
    hm->entries[next_slot].value = NULL;

    current_slot = next_slot;
  }
}

hash_map_entry* __hm_find_slot(hash_map* hm, void* key, bool allow_empty, size_t* out_slot) {
  uint64_t hash = hm->hash_fn(key);
  size_t original_slot = hash % hm->capacity;
  size_t current_slot = original_slot;
  while (true) {
    hash_map_entry* entry = &hm->entries[current_slot];
    if (entry->key == NULL) {
      if (allow_empty) {
        if (out_slot != NULL) {
          *out_slot = current_slot;
        }
        return entry;
      }

      return NULL;
    } else {
      // if the entry is our hash, return the entry
      if (entry->hash == hash) {
        if (out_slot != NULL) {
          *out_slot = current_slot;
        }
        return entry;
      }

      // if the new slot is correctly placed, the linear chain is broken. return
      // null and the caller needs to decide how to proceed
      if (current_slot != original_slot && hm->hash_fn(hm->entries[current_slot].key) % hm->capacity == current_slot) {
        return NULL;
      }

      // otherwise we need to linearly probe through the rest of the list to find
      // either the next empty spot (element not found) or the correct entry (hash matches)
      current_slot++;

      // if we've overflowed our capacity, roll around to the front 
      if (current_slot >= hm->capacity) {
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

void __hm_grow(hash_map* hm) {
  hash_map new_hm = {
    .hash_fn = hm->hash_fn,
    .load_factor = hm->load_factor,
    .entries = (hash_map_entry*)calloc(hm->capacity * 2, sizeof(hash_map_entry)),
    .length = 0,
    .capacity = hm->capacity * 2,
  };

  // move all existing entries to the new allocation;
  for (size_t i = 0; i < hm->capacity; i++) {
    if (hm->entries[i].key != NULL) {
      hm_insert(&new_hm, hm->entries[i].key, hm->entries[i].value);
    }
  }

  // now swap our hash_maps
  hash_map old_hm = *hm;
  *hm = new_hm;

  // and clean up our old allocation
  free(old_hm.entries);
}

#endif // OASIS_C_COLLECTIONS_HASH_MAP_H
