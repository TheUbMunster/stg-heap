#include "Heap.hpp"
#include <stdio.h>
#include <stdlib.h>

int main()
{
	printf("before heap construction");
	STGHeap heap{};
	printf("after heap construction");
	size_t* var = (size_t*)heap.stg_malloc(sizeof(size_t));
	void* var2 = heap.stg_malloc(24);
	heap.stg_free(var2);
	//test
	//size_t s = 100000;
	//void* a = p_alloc(s);
	//printf("%p", a);
	//p_free(a, s);
}