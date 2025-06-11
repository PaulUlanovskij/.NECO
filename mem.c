#include <assert.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "headers/cio.h"
#include "headers/mem.h"

void *malloc_copy(int size, void *src) {
  void *cpy = malloc(size);
  memcpy(cpy, src, size);
  return cpy;
}

void arena_init(ArenaAllocator *arena, int kilobytes) {
  if (arena_is_initialized(arena) == false) {
    panic("Arena was uninitialized");
  }

  arena->base_ptr = malloc(kilobytes * 1024);
  if (arena->base_ptr == NULL) {
    panic("Malloc failed");
  }

  arena->alloc_ptr = (unsigned char *)arena->base_ptr;
  arena->capacity = kilobytes * 1024;
}

char arena_is_initialized(ArenaAllocator *arena) {
  return !(arena == NULL || arena->base_ptr == NULL || arena->capacity == 0);
}

void arena_flush(ArenaAllocator *arena) {
  if (arena_is_initialized(arena) == false) {
    panic("Arena was uninitialized");
  }

  arena->alloc_ptr = arena->base_ptr;
}
void arena_deinit(ArenaAllocator *arena) {
  if (arena_is_initialized(arena) == false) {
    panic("Arena was uninitialized");
  }

  free(arena->base_ptr);
  arena->alloc_ptr = 0;
  arena->capacity = 0;
}

void *ptr_align(void *ptr, uintptr_t align) {
  return (void *)(((uintptr_t)ptr + align - 1) & ~(align - 1));
}

void *arena_try_alloc_bytes(ArenaAllocator *arena, uintptr_t size,
                            uintptr_t align) {
  if (arena_is_initialized(arena) == false) {
    panic("Arena was uninitialized");
  }

  void *ptr_with_align = ptr_align(arena->alloc_ptr, align);
  uintptr_t after_alloc_ptr = ((uintptr_t)ptr_with_align + size);
  uintptr_t max_arena_ptr = (uintptr_t)arena->base_ptr + arena->capacity;

  if (max_arena_ptr < after_alloc_ptr) {
    void *temp = arena->alloc_ptr;
    arena->alloc_ptr = (void *)after_alloc_ptr;
    return temp;
  }
  log_warning("Ran out of space on arena.");
  return NULL;
}
