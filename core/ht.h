#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "str.h"

#define define_simple_ht(key_type, value_type, name)\
typedef struct{\
  uint64_t (*hashfunc)(key_type);\
  bool (*eqlfunc)(key_type, key_type);\
  int length;\
  int capacity;\
  value_type* values;\
  key_type* keys;\
  bool* occupied;\
}name;
//TODO: add limit on hash table cycling

#define ht_init(ht, cap, hash_func, eql_func)                                  \
do {                                                                           \
  (ht)->hashfunc = hash_func;                                                  \
  (ht)->eqlfunc = eql_func;                                                    \
  (ht)->length = 0;                                                            \
  (ht)->capacity = (cap);                                                      \
  (ht)->values = (typeof((ht)->values))calloc((cap), sizeof(*(ht)->values));   \
  (ht)->keys = (typeof((ht)->keys))calloc((cap), sizeof(*(ht)->keys));         \
  (ht)->occupied = (bool*)calloc((cap), sizeof(bool));                         \
}while(0)

#define ht_set(ht, key, item) _ht_set(ht, key, item, __LINE__)
#define _ht_set(ht, key, item, line) __ht_set(ht, key, item, line)
#define __ht_set(ht, key, item, line)                                          \
do {                                                                           \
  __ht_reserve((ht), (ht)->length+1, line);                                            \
  int _ht_set_hash_index##line = (ht)->hashfunc(key) % (ht)->capacity;         \
  while((ht)->occupied[_ht_set_hash_index##line]){                             \
  if((ht)->eqlfunc(key, (ht)->keys[_ht_set_hash_index##line])){                \
      (ht)->values[_ht_set_hash_index##line] = item;                           \
      goto _ht_set_short_circuit_label##line;                                  \
    }                                                                          \
    _ht_set_hash_index##line = (_ht_set_hash_index##line+1) % (ht)->capacity;  \
  }                                                                            \
  (ht)->values[_ht_set_hash_index##line] = item;                               \
  (ht)->keys[_ht_set_hash_index##line] = key;                                  \
  (ht)->occupied[_ht_set_hash_index##line] = true;                             \
  (ht)->length++;                                                              \
_ht_set_short_circuit_label##line:                                             \
}while(0)

#define ht_get(ht, key, itemp) _ht_get(ht, key, itemp, __LINE__)
#define _ht_get(ht, key, itemp, line) __ht_get(ht, key, itemp, line)
#define __ht_get(ht, key, itemp, line)                                         \
do{                                                                            \
  int _ht_get_hash_index##line = (ht)->hashfunc(key) % (ht)->capacity;         \
  while((ht)->occupied[_ht_get_hash_index##line]){                             \
    if((ht)->eqlfunc(key, (ht)->keys[_ht_get_hash_index##line])){              \
      itemp = &(ht)->values[_ht_get_hash_index##line];                         \
      goto _ht_get_short_circuit_label##line;                                  \
    }                                                                          \
    _ht_get_hash_index##line = (_ht_get_hash_index##line+1) % (ht)->capacity;  \
  }                                                                            \
  itemp = nullptr;                                                             \
_ht_get_short_circuit_label##line:                                             \
}while(0)

#define ht_reserve(ht, cap) _ht_reserve(ht, cap, __LINE__)                                                    
#define _ht_reserve(ht, cap, line) __ht_reserve(ht, cap, line)                                                    
#define __ht_reserve(ht, cap, line)                                            \
do {                                                                           \
  if ((ht)->capacity < (cap)) {                                                \
    (ht)->capacity = cap - 1;                                                  \
    (ht)->capacity |= (ht)->capacity >> 1;                                     \
    (ht)->capacity |= (ht)->capacity >> 2;                                     \
    (ht)->capacity |= (ht)->capacity >> 4;                                     \
    (ht)->capacity |= (ht)->capacity >> 8;                                     \
    (ht)->capacity |= (ht)->capacity >> 16;                                    \
    (ht)->capacity++;                                                          \
                                                                               \
    free((ht)->occupied);                                                      \
    (ht)->occupied = (bool*)calloc((ht)->capacity, sizeof(bool));              \
                                                                               \
    typeof((ht)->values) _ht_old_values##line = (ht)->values;                  \
    (ht)->values = (typeof((ht)->values))calloc((ht)->capacity, sizeof(*(ht)->values));\
    typeof((ht)->keys) _ht_old_keys##line = (ht)->keys;                        \
    (ht)->keys = (typeof((ht)->keys))calloc((ht)->capacity, sizeof(*(ht)->keys));\
                                                                               \
    for(int _ht_reserve_i##line = 0;                                           \
        _ht_reserve_i##line < (ht)->length;                                    \
        _ht_reserve_i##line++){                                                \
      (ht)->length--;                                                          \
      int _ht_set_hash_index##line = (ht)->hashfunc(_ht_old_keys##line[_ht_reserve_i##line]) % (ht)->capacity;     \
      while((ht)->occupied[_ht_set_hash_index##line]){                         \
        _ht_set_hash_index##line = (_ht_set_hash_index##line+1) % (ht)->capacity;\
      }                                                                        \
      (ht)->values[_ht_set_hash_index##line] = _ht_old_values##line[_ht_reserve_i##line];                           \
      (ht)->keys[_ht_set_hash_index##line] = _ht_old_keys##line[_ht_reserve_i##line];                              \
      (ht)->occupied[_ht_set_hash_index##line] = true;                         \
      (ht)->length++;                                                          \
    }                                                                          \
    free(_ht_old_keys##line);                                                  \
    free(_ht_old_values##line);                                                \
  }                                                                            \
} while (0)

#define ht_for(ht, index, key, value)\
for(int index = 0; index < (ht)->capacity;)\
  for(typeof(*(ht)->keys) key = (ht)->keys[0]; index < (ht)->capacity;)\
    for(typeof(*(ht)->values) value = (ht)->values[0]; index < (ht)->capacity; index++, key = (ht)->keys[index], value = (ht)->values[index])\
      if((ht)->occupied[index])

uint64_t djb2(void* mem, int size);

#define simphash(x) djb2((void*)x, sizeof(x))

#define hash(x) _Generic(x,                                                    \
  cstr: cstr_hash,                                                             \
  vstr: vstr_hash,                                                             \
  dstr: dstr_hash,                                                             \
  cstr_da: cstr_da_hash,                                                       \
  vstr_da: vstr_da_hash,                                                       \
  bool: bool_hash,                                                             \
  char: char_hash,                                                             \
  short: short_hash,                                                           \
  int: int_hash,                                                               \
  long: long_hash,                                                             \
  float: float_hash,                                                           \
  double: double_hash,                                                         \
  default: simphash                                                            \
)(x)

static inline uint64_t bool_hash(bool x);
static inline uint64_t char_hash(char x);
static inline uint64_t short_hash(short x);
static inline uint64_t int_hash(int x);
static inline uint64_t long_hash(long x);
static inline uint64_t float_hash(float x);
static inline uint64_t double_hash(double x);

uint64_t cstr_hash(cstr str);
uint64_t vstr_hash(vstr str);
uint64_t dstr_hash(dstr str);
uint64_t cstr_da_hash(cstr_da da);
uint64_t vstr_da_hash(vstr_da da);
