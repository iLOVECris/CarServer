#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cache_allocer.h"
#define ELEMENT_SIZE 128
#define ELEMENT_COUNT 1024
static struct cache_allocer* small_allocer;

void* small_alloc(int size)
{
	if(small_allocer==NULL)
	{
		small_allocer = create_cache_allocer(ELEMENT_COUNT, ELEMENT_SIZE);
	}
	return cache_alloc(small_allocer, size);
}
void free_small_alloc(void* memory)
{
	if (small_allocer == NULL)
	{
		small_allocer = create_cache_allocer(ELEMENT_COUNT, ELEMENT_SIZE);
	}
	cache_free(small_allocer,memory);
}
