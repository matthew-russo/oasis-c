#include <cstring>
#include <gtest/gtest.h>
#include <stdint.h>

#include "hash/fnv.h"

typedef struct {
  char* input;
  uint64_t expected_hash;
} hash_input;

TEST(HashFnvTest, SmokeTest) {
  hash_input cases[3] = {
    { .input = (char*)"hello world", .expected_hash = 8618312879776256743ULL, },
    { .input = (char*)"test", .expected_hash = 18007334074686647077ULL, },
    { .input = (char*)"hashing", .expected_hash = 9282440586321907183ULL, },
  };

  int len = sizeof(cases) / sizeof(cases[0]);
  for (int i = 0; i < len; i++) {
    uint64_t hash = fnv1a_64_bytes((uint8_t*)cases[i].input, strlen(cases[i].input));
    EXPECT_EQ(cases[i].expected_hash, hash);
  }
}

