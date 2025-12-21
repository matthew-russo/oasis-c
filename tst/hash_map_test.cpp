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

TEST(CollectionsHashMapTest, HashMapInitDestroy) {
  hash_map* hm = hm_create(str_hash_fn, 0.8);
  hm_destroy(hm);
}

TEST(CollectionsHashMapTest, HashMapIsEmpty) {
  hash_map* hm = hm_create(str_hash_fn, 0.8);
  EXPECT_TRUE(hm_is_empty(hm));
  hm_destroy(hm);
}

TEST(CollectionsHashMapTest, HashMapGetNonExistent) {
  hash_map* hm = hm_create(str_hash_fn, 0.8);
  hash_map_entry* entry = hm_get(hm, (void*)"hello");
  EXPECT_EQ(nullptr, entry);
  ASSERT_EQ(0, hm->length);
  hm_destroy(hm);
}

TEST(CollectionsHashMapTest, HashMapInsert) {
  hash_map* hm = hm_create(str_hash_fn, 0.8);
  hm_insert(hm, (void*)"hello", (void*)"world");
  ASSERT_EQ(1, hm->length);
  hm_destroy(hm);
}

TEST(CollectionsHashMapTest, HashMapInsertGet) {
  hash_map* hm = hm_create(str_hash_fn, 0.8);
  hm_insert(hm, (void*)"hello", (void*)"world");
  ASSERT_EQ(1, hm->length);
  hash_map_entry* entry = hm_get(hm, (void*)"hello");
  EXPECT_NE(nullptr, entry);
  EXPECT_STREQ((char*)entry->key, "hello");
  EXPECT_STREQ((char*)entry->value, "world");
  hm_destroy(hm);
}

TEST(CollectionsHashMapTest, HashMapInsertRemove) {
  hash_map* hm = hm_create(str_hash_fn, 0.8);
  hm_insert(hm, (void*)"hello", (void*)"world");
  ASSERT_EQ(1, hm->length);
  hm_remove(hm, (void*)"hello");
  ASSERT_EQ(0, hm->length);
  ASSERT_TRUE(hm_is_empty(hm));
  hm_destroy(hm);
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
  hm_destroy(hm);
}

TEST(CollectionsHashMapTest, HashMapWrapsAround) {
  hash_map* hm = hm_create(str_hash_fn, 0.8);

  // this test validates that linear probing properly wraps around to the front
  // of the array. both keys -- "19" and "20" -- hash to a value, which when modulo'ed
  // with the default capacity of 32, result in both being placed in slot 31 (the
  // last slot). we expect key1 to be in slot 31 and key2 to be in slot 0.

  char const* key1 = "19";
  char const* key2 = "20";

  // ensure we're actually testing what we think we're testing
  ASSERT_EQ(HM_DEFAULT_CAPACITY - 1, str_hash_fn((void*)key1) % HM_DEFAULT_CAPACITY);
  ASSERT_EQ(HM_DEFAULT_CAPACITY - 1, str_hash_fn((void*)key2) % HM_DEFAULT_CAPACITY);

  hm_insert(hm, (void*)key1, (void*)key1);
  hm_insert(hm, (void*)key2, (void*)key2);

  ASSERT_EQ(2, hm->length);

  hash_map_entry entry1 = hm->entries[HM_DEFAULT_CAPACITY - 1];
  EXPECT_EQ(entry1.hash, str_hash_fn((void*)key1));
  EXPECT_STREQ((char*)entry1.key, key1);
  EXPECT_STREQ((char*)entry1.value, key1);

  hash_map_entry entry2 = hm->entries[0];
  EXPECT_EQ(entry2.hash, str_hash_fn((void*)key2));
  EXPECT_STREQ((char*)entry2.key, key2);
  EXPECT_STREQ((char*)entry2.value, key2);
}
