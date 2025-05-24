#pragma once
#include "helpers.h"
#include <stdalign.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  void *base_ptr;
  unsigned char *alloc_ptr;
  int capacity;
} ArenaAllocator;

void arena_init(ArenaAllocator *arena, int kilobytes) {
  assert(arena != NULL);
  assert(arena->base_ptr == NULL);
  assert(kilobytes != 0);

  arena->base_ptr = malloc(kilobytes * 1024);
  assert(arena->base_ptr != NULL);

  arena->alloc_ptr = (unsigned char *)arena->base_ptr;
  arena->capacity = kilobytes * 1024;
}

pubool arena_is_initialized(ArenaAllocator *arena) {
  return !(arena == NULL || arena->base_ptr == NULL || arena->capacity == 0);
}

void arena_flush(ArenaAllocator *arena) {
  assert(arena_is_initialized(arena));

  arena->alloc_ptr = (unsigned char *)arena->base_ptr;
}
void arena_deinit(ArenaAllocator *arena) {
  assert(arena_is_initialized(arena));

  free(arena->base_ptr);
  arena->alloc_ptr = 0;
  arena->capacity = 0;
}

uintptr_t ptr_align(uintptr_t ptr, uintptr_t align) {
  return (ptr + align - 1) & ~(align - 1);
}

pubool arena_has_space_for(ArenaAllocator *arena, int size, uintptr_t align,
                         int len) {
  assert(arena_is_initialized(arena));
  assert(size != 0);
  assert(align != 0);
  assert(len != 0);

  uintptr_t size_with_align = ptr_align(size, align);
  uintptr_t aligned_alloc_prt = ptr_align((uintptr_t)arena->alloc_ptr, align);

  return (aligned_alloc_prt + size_with_align * len <
          (uintptr_t)arena->base_ptr + arena->capacity);
}

static void *_arena_alloc(ArenaAllocator *arena, uintptr_t size,
                          uintptr_t align) {
  assert(arena_is_initialized(arena));
  assert(size != 0);
  assert(align != 0);

  if (arena_has_space_for(arena, size, align, 1) == false) {
    return NULL;
  }
  arena->alloc_ptr =
      (unsigned char *)ptr_align((uintptr_t)arena->alloc_ptr, align);
  void *pointer = (void *)arena->alloc_ptr;
  memset(pointer, 0, size);
  arena->alloc_ptr += size;
  return pointer;
}
static void *_arena_alloc_array(ArenaAllocator *arena, uintptr_t size,
                                uintptr_t align, uintptr_t len) {
  assert(arena_is_initialized(arena));
  assert(size != 0);
  assert(align != 0);
  assert(len != 0);

  if (arena_has_space_for(arena, size, align, len) == false) {
    return NULL;
  }
  uintptr_t size_with_align = ptr_align(size, align);
  return _arena_alloc(arena, size_with_align * len, align);
}
#define arena_alloc(T, arena)                                                  \
  do {                                                                         \
    (faulty_ptr(ArenaError, T)) _t =                                           \
        _arena_alloc(arena, sizeof(T), alignof(T));                            \
    (faulty_ptr(ArenaError, T)){.is_error = _t.is_error,                       \
                                .value = (T *)_t.value};                       \
  } while (0);
#define arena_alloc_array(T, arena, len)                                       \
  do {                                                                         \
    faulty_ptr(ArenaError, void) _t =                                          \
        _arena_alloc_array(arena, sizeof(T), alignof(T), len);                 \
    (faulty_ptr(ArenaError, T)){.is_error = _t.is_error,                       \
                                .value = (T *)_t.value};                       \
  } while (0);
