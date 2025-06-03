#pragma once
#include "types.h"
#include <stdint.h>

void *malloc_copy(int size, void *src);

void arena_init(ArenaAllocator *arena, int kilobytes);
char arena_is_initialized(ArenaAllocator *arena);

void arena_flush(ArenaAllocator *arena);
void arena_deinit(ArenaAllocator *arena);

void* ptr_align(void* ptr, uintptr_t align);
void *arena_try_alloc_bytes(ArenaAllocator *arena, uintptr_t size,
                            uintptr_t align);

#define arena_try_alloc(arena, T)                                              \
  (T *)arena_try_alloc((arena), sizeof((T)), alignof((T)))
#define arena_try_alloc_array(arena, T, size)                                  \
  (T *)arena_try_alloc((arena), sizeof((T)) * (size), alignof((T)))


