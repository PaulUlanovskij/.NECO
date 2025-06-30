#include <assert.h>
#include <stdalign.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cio.h"
#include "mem.h"

void *malloc_copy(int size, void *src) {
  void *cpy = malloc(size);
  memcpy(cpy, src, size);
  return cpy;
}
void *ptr_align(void *ptr, uintptr_t align) {
  return (void *)(((uintptr_t)ptr + align - 1) & ~(align - 1));
}

void scratch_arena_init(ScratchArenaAllocator *arena, int kilobytes) {
  if (scratch_arena_is_initialized(arena) == false) {
    panic("Arena was uninitialized");
  }

  arena->base_ptr = malloc(kilobytes * 1024);
  if (arena->base_ptr == NULL) {
    panic("Malloc failed");
  }

  arena->alloc_ptr = (unsigned char *)arena->base_ptr;
  arena->capacity = kilobytes * 1024;
}

char scratch_arena_is_initialized(ScratchArenaAllocator *arena) {
  return !(arena == NULL || arena->base_ptr == NULL || arena->capacity == 0);
}

void scratch_arena_flush(ScratchArenaAllocator *arena) {
  if (scratch_arena_is_initialized(arena) == false) {
    panic("Arena was uninitialized");
  }

  arena->alloc_ptr = arena->base_ptr;
}
void scratch_arena_deinit(ScratchArenaAllocator *arena) {
  if (scratch_arena_is_initialized(arena) == false) {
    panic("Arena was uninitialized");
  }

  free(arena->base_ptr);
  arena->alloc_ptr = 0;
  arena->capacity = 0;
}

void *scratch_arena_alloc_bytes(ScratchArenaAllocator *arena, uintptr_t size, uintptr_t align) {
  if (scratch_arena_is_initialized(arena) == false) {
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
  panic("Arena ran out of space.");
}

void mempage_init(MemPage *page){
  if((page->base = malloc(MEMPAGE_SIZE)) == NULL){
    puts("Malloc failed");
    exit(1);
  }
  page->alloc_indices = (int_da){};
  page->alloc_sizes = (int_da){};
}

void mempage_deinit(MemPage *page){
  da_free(&page->alloc_indices);
  da_free(&page->alloc_sizes);
  free(page->base);
}
void* mempage_alloc(MemPage *page, uintptr_t size, uintptr_t align){
  int index = 0;
  da_for(&page->alloc_indices, i, start){
    if(index == *start){
      index += page->alloc_sizes.items[i];
      continue;
    }
    int hole_size = *start - index;
    char* aligned_ptr = (char*)ptr_align((char*)page->base + index, align);
    int effective_size = (int)((char*)page->base + *start - aligned_ptr);
    if(effective_size >= size){
      da_append(&page->alloc_indices, index);
      da_append(&page->alloc_sizes, size);
      return aligned_ptr;
    }
  }
  return nullptr;
}
bool mempage_free(MemPage *page, void* ptr){
  da_for(&page->alloc_indices, i, start){
    if(ptr == (char*)page->base + *start){
      da_remove_at_ord(&page->alloc_sizes, j, i);
      da_remove_at_ord(&page->alloc_indices, j, i);
      return true;
    }
  }
  return false;
}

void *arena_alloc_bytes(ArenaAllocator *arena, uintptr_t size, uintptr_t align){
  void* ptr = nullptr;
  da_foreach(arena, page){
    if((ptr = mempage_alloc(page, size, align)) != NULL){
      return ptr;
    }
  }
  MemPage page = {};
  mempage_init(&page);
  da_append(arena, page);
  return mempage_alloc(&page, size, align);
}
void arena_free(ArenaAllocator *arena, void* ptr){
  da_foreach(arena, page){
    if(mempage_free(page, ptr)){
      return;
    }
  }
}
void arena_deinit(ArenaAllocator *arena){
  da_foreach(arena, page){
    mempage_deinit(page);
  }
  da_free(arena);
}

