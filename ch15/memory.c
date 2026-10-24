#include <stdlib.h>
#include <string.h>

#include "memory.h"

#define HEAP_SIZE (16 * 1024 * 1024)

typedef struct Block {
  size_t size;
  bool free;
  struct Block* next;
} Block;

static Block* heap = NULL;

static size_t align(size_t size) {
  return (size + 7) & ~(size_t)7;
}

static void initHeap(void) {
  heap = malloc(HEAP_SIZE);
  if (heap == NULL) exit(1);
  heap->size = HEAP_SIZE - sizeof(Block);
  heap->free = true;
  heap->next = NULL;
}

static void split(Block* block, size_t size) {
  if (block->size < size + sizeof(Block) + 8) return;

  Block* rest = (Block*)((char*)(block + 1) + size);
  rest->size = block->size - size - sizeof(Block);
  rest->free = true;
  rest->next = block->next;

  block->size = size;
  block->next = rest;
}

static void* allocate(size_t size) {
  if (heap == NULL) initHeap();
  size = align(size);

  for (Block* block = heap; block != NULL; block = block->next) {
    if (block->free && block->size >= size) {
      split(block, size);
      block->free = false;
      return block + 1;
    }
  }

  exit(1);
}

static void coalesce(void) {
  Block* block = heap;
  while (block != NULL && block->next != NULL) {
    if (block->free && block->next->free) {
      block->size += sizeof(Block) + block->next->size;
      block->next = block->next->next;
    } else {
      block = block->next;
    }
  }
}

static void release(void* pointer) {
  Block* block = (Block*)pointer - 1;
  block->free = true;
  coalesce();
}

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
  if (newSize == 0) {
    if (pointer != NULL) release(pointer);
    return NULL;
  }

  if (pointer == NULL) return allocate(newSize);

  Block* block = (Block*)pointer - 1;
  if (block->size >= newSize) return pointer;

  void* result = allocate(newSize);
  memcpy(result, pointer, oldSize < newSize ? oldSize : newSize);
  release(pointer);
  return result;
}

void freeHeap(void) {
  free(heap);
  heap = NULL;
}
