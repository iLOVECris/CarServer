#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "../utils/cache_allocer.h"

struct cache_node
{
	struct cache_node* next;
};

struct cache_allocer
{
	int capacity;
	int elem_size;
	struct cache_node* free_list;
	char* cache_memory;
};
struct cache_allocer* create_cache_allocer(int capacity, int elem_size)
{
	elem_size < sizeof(struct cache_node) ? sizeof(struct cache_node) : elem_size;
	struct cache_allocer* allocer = malloc(sizeof(struct cache_allocer));
	memset(allocer, 0, sizeof(struct cache_allocer));
	allocer->capacity = capacity;
	allocer->elem_size = elem_size;
	allocer->cache_memory = malloc(capacity*elem_size);
	allocer->free_list = NULL;
	for (int index = 0; index < capacity; index++)
	{
		struct cache_node* node = (struct cahce_node*)(allocer->cache_memory + elem_size*index);
		node->next = allocer->free_list;
		allocer->free_list = node;
	}
	return allocer;
}
void destroy_cache_allocer(struct cache_allocer* allocer)
{
	if (allocer->cache_memory!=NULL)
	{
		free(allocer->cache_memory);
	}

	free(allocer);
}
void* cache_alloc(struct cache_allocer* allocer, int elem_size)
{
	if (allocer->elem_size < elem_size)
	{
		return malloc(elem_size);
	}
	if (allocer->free_list != NULL)
	{
		char* mem = allocer->free_list;
		allocer->free_list = allocer->free_list->next;
		return mem;
	}
	return malloc(elem_size);
}
void cache_free(struct cache_allocer* allocer, void* mem)
{
	if (mem >= allocer->cache_memory && mem < (allocer->cache_memory + allocer->capacity*allocer->elem_size))//是内存池中内存不足额外申请的内存
	{
		struct cache_node* node = mem;
		node->next = allocer->free_list;
		allocer->free_list = node;
		return;
	}
	free(mem);
}