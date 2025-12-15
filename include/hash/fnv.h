#ifndef OASIS_C_HASH_FNV_H
#define OASIS_C_HASH_FNV_H

#include <stdint.h>
#include <stddef.h>

#define FNV_OFFSET_BASIS32 0x811c9dc5UL
#define FNV_OFFSET_BASIS64 0xcbf29ce484222325ULL
#define FNV_PRIME32        0x01000193UL
#define FNV_PRIME64        0x100000001b3ULL

// ============ 32-bit FNV-1 hash ============

/// Calculates the 32-bit FNV-1 hash of a given byte buffer, `buf`, with length `len`.
///
/// `hash` is used as the starting hash value, allowing hash computations across multiple
/// buffers. if you have a single buffer, use `fnv1_32_bytes` which properly initalizes
/// the hash to `FNV_OFFSET_BASIS32`
inline uint32_t fnv1_32_bytes_incr(uint32_t hash, uint8_t* buf, size_t len) {
  for (size_t i = 0; i < len; i++) {
    hash *= FNV_PRIME32;
    hash ^= buf[i];
  }
  return hash;
}

/// Calculates the 32-bit FNV-1 hash of a given byte buffer, `buf`, with length `len`.
inline uint32_t fnv1_32_bytes(uint8_t* buf, size_t len) {
  return fnv1_32_bytes_incr(FNV_OFFSET_BASIS32, buf, len);
}

// ============ 64-bit FNV-1 hash ============

/// Calculates the 64-bit FNV-1 hash of a given byte buffer, `buf`, with length `len`.
///
/// `hash` is used as the starting hash value, allowing hash computations across multiple
/// buffers. if you have a single buffer, use `fnv1_64_bytes` which properly initalizes
/// the hash to `FNV_OFFSET_BASIS64`
inline uint64_t fnv1_64_bytes_incr(uint64_t hash, uint8_t* buf, size_t len) {
  for (size_t i = 0; i < len; i++) {
    hash *= FNV_PRIME64;
    hash ^= buf[i];
  }
  return hash;
}

/// Calculates the 64-bit FNV-1 hash of a given byte buffer, `buf`, with length `len`.
inline uint64_t fnv1_64_bytes(uint8_t* buf, size_t len) {
  return fnv1_64_bytes_incr(FNV_OFFSET_BASIS64, buf, len);
}

// 32-bit FNV-1a hash

/// Calculates the 32-bit FNV-1a hash of a given byte buffer, `buf`, with length `len`.
///
/// FNV-1a has better avalanche characteristics than FNV-1
///
/// `hash` is used as the starting hash value, allowing hash computations across multiple
/// buffers. if you have a single buffer, use `fnv1_64_bytes` which properly initalizes
/// the hash to `FNV_OFFSET_BASIS64`
inline uint32_t fnv1a_32_bytes_incr(uint32_t hash, uint8_t* buf, size_t len) {
   for (size_t i = 0; i < len; i++) {
    hash ^= buf[i];
    hash *= FNV_PRIME32;
  }
  return hash; 
}

/// Calculates the 32-bit FNV-1a hash of a given byte buffer, `buf`, with length `len`.
///
/// FNV-1a has better avalanche characteristics than FNV-1
inline uint32_t fnv1a_32_bytes(uint8_t* buf, size_t len) {
  return fnv1a_32_bytes_incr(FNV_OFFSET_BASIS32, buf, len);
}

// 64-bit FNV-1a hash

/// Calculates the 64-bit FNV-1a hash of a given byte buffer, `buf`, with length `len`.
///
/// FNV-1a has better avalanche characteristics than FNV-1
///
/// `hash` is used as the starting hash value, allowing hash computations across multiple
/// buffers. if you have a single buffer, use `fnv1_64_bytes` which properly initalizes
/// the hash to `FNV_OFFSET_BASIS64`
inline uint64_t fnv1a_64_bytes_incr(uint64_t hash, uint8_t* buf, size_t len) {
  for (size_t i = 0; i < len; i++) {
    hash ^= buf[i];
    hash *= FNV_PRIME64;
  }
  return hash;
}

/// Calculates the 64-bit FNV-1a hash of a given byte buffer, `buf`, with length `len`.
///
/// FNV-1a has better avalanche characteristics than FNV-1
inline uint64_t fnv1a_64_bytes(uint8_t* buf, size_t len) {
  return fnv1a_64_bytes_incr(FNV_OFFSET_BASIS64, buf, len);
}

#endif // OASIS_C_HASH_FNV_H
