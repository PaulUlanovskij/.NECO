#pragma once
#include <stdint.h>

#include "da.h"

typedef struct {
  void *base_ptr;
  void *alloc_ptr;
  int capacity;
} ScratchArenaAllocator;

#define MEMPAGE_SIZE 4096
typedef struct {
  void* base;
  int_da alloc_indices;
  int_da alloc_sizes;
}MemPage;
typedef struct {
  MemPage* items;
  int length;
  int capacity;
} ArenaAllocator;

void *malloc_copy(int size, void *src);
void* ptr_align(void* ptr, uintptr_t align);

void scratch_arena_init(ScratchArenaAllocator *arena, int kilobytes);
char scratch_arena_is_initialized(ScratchArenaAllocator *arena);

void scratch_arena_flush(ScratchArenaAllocator *arena);
void scratch_arena_deinit(ScratchArenaAllocator *arena);

void *scratch_arena_try_alloc_bytes(ScratchArenaAllocator *arena, uintptr_t size, uintptr_t align);

#define scratch_arena_alloc(arena, T)                                              \
  (T *)scratch_arena_alloc_bytes((arena), sizeof((T)), alignof((T)))
#define scratch_arena_alloc_array(arena, T, size)                                  \
  (T *)scratch_arena_alloc_bytes((arena), sizeof((T)) * (size), alignof((T)))

void *arena_alloc_bytes(ArenaAllocator *arena, uintptr_t size, uintptr_t align);
void arena_free(ArenaAllocator *arena, void* ptr);
void arena_deinit(ArenaAllocator *arena);

#define arena_alloc(arena, T)                                              \
  (T *)arena_alloc_bytes((arena), sizeof((T)), alignof((T)))
#define arena_alloc_array(arena, T, size)                                  \
  (T *)arena_alloc_bytes((arena), sizeof((T)) * (size), alignof((T)))

