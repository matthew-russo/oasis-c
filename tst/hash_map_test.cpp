#include <cstring>
#include <gtest/gtest.h>
#include <stdint.h>

#include "collections/hash_map.h"
#include "hash/fnv.h"

typedef struct {
  char* input;
  uint64_t expected_hash;
} hash_input;

uint64_t str_hash_fn(void* opaque_key) {
  char* key = (char*)opaque_key;
  size_t len = strlen(key);
  return fnv1a_64_bytes((uint8_t*)key, len);
}

TEST(CollectionsHashMapTest, HashMapInit) {
  hash_map* hm = hm_create(str_hash_fn, 0.8);
}

TEST(CollectionsHashMapTest, HashMapIsEmpty) {
  hash_map* hm = hm_create(str_hash_fn, 0.8);
  EXPECT_TRUE(hm_is_empty(hm));
}

TEST(CollectionsHashMapTest, HashMapGetNonExistent) {
  hash_map* hm = hm_create(str_hash_fn, 0.8);
  hash_map_entry* entry = hm_get(hm, (void*)"hello");
  EXPECT_EQ(nullptr, entry);
  ASSERT_EQ(0, hm->length);
}

TEST(CollectionsHashMapTest, HashMapInsert) {
  hash_map* hm = hm_create(str_hash_fn, 0.8);
  hm_insert(hm, (void*)"hello", (void*)"world");
  ASSERT_EQ(1, hm->length);
}

TEST(CollectionsHashMapTest, HashMapInsertGet) {
  hash_map* hm = hm_create(str_hash_fn, 0.8);
  hm_insert(hm, (void*)"hello", (void*)"world");
  ASSERT_EQ(1, hm->length);
  hash_map_entry* entry = hm_get(hm, (void*)"hello");
  EXPECT_NE(nullptr, entry);
  EXPECT_STREQ((char*)entry->key, "hello");
  EXPECT_STREQ((char*)entry->value, "world");
}

TEST(CollectionsHashMapTest, HashMapInsertRemove) {
  hash_map* hm = hm_create(str_hash_fn, 0.8);
  hm_insert(hm, (void*)"hello", (void*)"world");
  ASSERT_EQ(1, hm->length);
  hm_remove(hm, (void*)"hello");
  ASSERT_EQ(0, hm->length);
  ASSERT_TRUE(hm_is_empty(hm));
}

TEST(CollectionsHashMapTest, HashMapInsertRemoveGet) {
  hash_map* hm = hm_create(str_hash_fn, 0.8);
  hm_insert(hm, (void*)"hello", (void*)"world");
  ASSERT_EQ(1, hm->length);
  hm_remove(hm, (void*)"hello");
  ASSERT_EQ(0, hm->length);
  ASSERT_TRUE(hm_is_empty(hm));
  hash_map_entry* entry = hm_get(hm, (void*)"hello");
  EXPECT_EQ(nullptr, entry);
}
