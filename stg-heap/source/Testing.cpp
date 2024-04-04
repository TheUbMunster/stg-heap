#include "../include/Heap.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <cstddef>

int main()
{
	printf("before heap construction");
	STGHeap heap{};
	printf("after heap construction");
	size_t* var = (size_t*)heap.stg_malloc(sizeof(size_t));
	*var = 0x6969696969696969ULL;
	void* var2 = heap.stg_malloc(24);
	for (size_t i = 0; i < 24; i++)
		*(((char*)var2) + i) = "Howdy! This is a thing."[i];
	heap.stg_free(var2);
}
