#ifndef ARENA_H
#define ARENA_H

#include <stdlib.h>

typedef struct {
	void* pointer;
	void* start;
	size_t taken;
	size_t size;
} Arena;

void arena_init(Arena* a, size_t size);
void* arena_alloc(Arena* a, size_t size);
void* arena_realloc(Arena* a, void* src, size_t prev_size, size_t size);

void arena_free(Arena* a);

#endif // ARENA_H
