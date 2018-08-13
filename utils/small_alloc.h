#ifndef __SMALL_ALLOC_H__
#define __SMALL_ALLOC_H__
#ifdef __cplusplus
extern "C"
{
#endif
	void* small_alloc(int size);
	void free_small_alloc(void* memory);
#ifdef  __cplusplus
}
#endif //  __cplusplus





#endif