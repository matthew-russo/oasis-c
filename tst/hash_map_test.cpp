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
  EXPECT_STREQ("hello", (char*)entry->key);
  EXPECT_STREQ("world", (char*)entry->value);
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
 
TEST(CollectionsHashMapTest, HashMapRemoveMaintainsLinearChains) {
  hash_map* hm = hm_create(str_hash_fn, 0.8);

  // this test validates that when removing elements that have been linearly chained,
  // the hash map properly shifts all remaining elements in the chain left, such
  // that there are no gaps.
  //
  // we have 3 keys ("19", "20", "55") that are predetermined to get placed in
  // slot 31. this will result in slots 31, 0, and 1 being filled with our linear
  // chain.
  //
  // we then also have a key ("3") that is predetermined to be in slot 2. such
  // that we have 4 elements in a row, but only 3 of them part of the chain. 
  //
  // when we remove "19", it needs to shift "20" from slot 0 to slot 31, properly
  // wrapping around and shift "55" from slot 1 to slot 0. it must then recognize
  // that key "3" in slot 2 is properly placed and *not* shift it.
  //
  // when we remove "20" now in slot 31, it needs to shift "55" from slot 0 to
  // slot 31.
  //
  // and then we can remove "55" and "3"

  char const* slot31_key1 = "19";
  char const* slot31_key2 = "20";
  char const* slot31_key3 = "55";
  char const* slot2_key1 = "3";

  // ensure we're actually testing what we think we're testing
  ASSERT_EQ(HM_DEFAULT_CAPACITY - 1, str_hash_fn((void*)slot31_key1) % HM_DEFAULT_CAPACITY);
  ASSERT_EQ(HM_DEFAULT_CAPACITY - 1, str_hash_fn((void*)slot31_key2) % HM_DEFAULT_CAPACITY);
  ASSERT_EQ(HM_DEFAULT_CAPACITY - 1, str_hash_fn((void*)slot31_key3) % HM_DEFAULT_CAPACITY);
  ASSERT_EQ(2, str_hash_fn((void*)slot2_key1) % HM_DEFAULT_CAPACITY);

  hm_insert(hm, (void*)slot31_key1, (void*)slot31_key1);
  hm_insert(hm, (void*)slot31_key2, (void*)slot31_key2);
  hm_insert(hm, (void*)slot31_key3, (void*)slot31_key3);
  hm_insert(hm, (void*)slot2_key1, (void*)slot2_key1);

  ASSERT_EQ(4, hm->length);

  hash_map_entry entry1;
  hash_map_entry entry2;
  hash_map_entry entry3;
  hash_map_entry entry4;

  entry1 = hm->entries[HM_DEFAULT_CAPACITY - 1];
  EXPECT_EQ(entry1.hash, str_hash_fn((void*)slot31_key1));
  EXPECT_STREQ(slot31_key1, (char*)entry1.key);
  EXPECT_STREQ(slot31_key1, (char*)entry1.value);

  entry2 = hm->entries[0];
  EXPECT_EQ(entry2.hash, str_hash_fn((void*)slot31_key2));
  EXPECT_STREQ(slot31_key2, (char*)entry2.key);
  EXPECT_STREQ(slot31_key2, (char*)entry2.value);

  entry3 = hm->entries[1];
  EXPECT_EQ(entry3.hash, str_hash_fn((void*)slot31_key3));
  EXPECT_STREQ(slot31_key3, (char*)entry3.key);
  EXPECT_STREQ(slot31_key3, (char*)entry3.value);

  entry4 = hm->entries[2];
  EXPECT_EQ(entry4.hash, str_hash_fn((void*)slot2_key1));
  EXPECT_STREQ(slot2_key1, (char*)entry4.key);
  EXPECT_STREQ(slot2_key1, (char*)entry4.value);

  // remove the first key, the 2 chained keys should be shifted left, the last key
  // should be untouched
  hm_remove(hm, (void*)slot31_key1);

  entry1 = hm->entries[HM_DEFAULT_CAPACITY - 1];
  EXPECT_EQ(entry1.hash, str_hash_fn((void*)slot31_key2));
  EXPECT_STREQ(slot31_key2, (char*)entry1.key);
  EXPECT_STREQ(slot31_key2, (char*)entry1.value);

  entry2 = hm->entries[0];
  EXPECT_EQ(entry2.hash, str_hash_fn((void*)slot31_key3));
  EXPECT_STREQ(slot31_key3, (char*)entry2.key);
  EXPECT_STREQ(slot31_key3, (char*)entry2.value);

  entry4 = hm->entries[2];
  EXPECT_EQ(entry4.hash, str_hash_fn((void*)slot2_key1));
  EXPECT_STREQ(slot2_key1, (char*)entry4.key);
  EXPECT_STREQ(slot2_key1, (char*)entry4.value);

  // remove the second key, the 1 chained keys should be shifted left, the last key
  // should be untouched
  hm_remove(hm, (void*)slot31_key2);

  entry1 = hm->entries[HM_DEFAULT_CAPACITY - 1];
  EXPECT_EQ(entry1.hash, str_hash_fn((void*)slot31_key3));
  EXPECT_STREQ(slot31_key3, (char*)entry1.key);
  EXPECT_STREQ(slot31_key3, (char*)entry1.value);

  entry4 = hm->entries[2];
  EXPECT_EQ(entry4.hash, str_hash_fn((void*)slot2_key1));
  EXPECT_STREQ(slot2_key1, (char*)entry4.key);
  EXPECT_STREQ(slot2_key1, (char*)entry4.value);

  hm_destroy(hm);
}
