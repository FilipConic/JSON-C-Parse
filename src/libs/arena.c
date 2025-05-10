#include "../include/arena.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

void arena_init(Arena* a, size_t size) {
	a->pointer = calloc(size, sizeof(char));
	if (!a->pointer) {
		fprintf(stderr, "ERROR: Unable to allocate %zu bytes!\nBuy more RAM!\n", size);
		return;
	}
	a->start = a->pointer;
	a->size = size;
	a->taken = 0;
}
void* arena_alloc(Arena* a, size_t size) {
	if (size + a->taken + 4 >= a->size || !a->start) {
		fprintf(stderr, "ERROR: Arena is full and its not possible to give you more memory!\n");
		return NULL;
	}
	void* ret = a->pointer;
	a->pointer = (void*)((intptr_t)a->pointer + size);
	a->taken += size;
	
	char mod = (intptr_t)a->pointer % 4;
	if (mod != 0) {
		a->pointer = (void*)((intptr_t)a->pointer + 4 - mod);
		a->taken += 4 - mod;
	}
	return ret;
}
void* arena_realloc(Arena* a, void* src, size_t prev_size, size_t size) {
	void* ret = arena_alloc(a, size);
	if (!ret) {
		return NULL;
	}
	if (size < prev_size) {
		memcpy(ret, src, size);
	} else {
		memcpy(ret, src, prev_size);
	}

	return ret;
}

void arena_free(Arena* a) {
	free(a->start);
	a->pointer = NULL;
	a->start = NULL;
	a->taken = 0;
	a->size = 0;
}
