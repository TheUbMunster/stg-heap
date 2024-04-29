#include "Heap.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <cstddef>
#include "gtest/gtest.h"

namespace
{
	TEST(SimpleTests, ConstructDeconstruct)
	{
		STGHeap heap{};
	}
	TEST(SimpleTests, SimpleAlloc)
	{
		STGHeap heap{};
		int* ptr = (int*)heap.stg_malloc(sizeof(int));
		EXPECT_NE(ptr, nullptr);
		*ptr = 5;
		EXPECT_EQ(*ptr, 5); //if this didn't pass, we would expect the previous line to segfault.
	}
	TEST(ComplexTests, SmallVariety)
	{
	        STGHeap heap{};
	        size_t* var = (size_t*)heap.stg_malloc(sizeof(size_t));
	        *var = 0x6969696969696969ULL;
	        void* var2 = heap.stg_malloc(24);
	        for (size_t i = 0; i < 24; i++)
	                *(((char*)var2) + i) = "Howdy! This is a thing."[i];
	        heap.stg_free(var2);
	        void* var3 = heap.stg_malloc(sizeof(size_t));
	        void* var4 = heap.stg_malloc(sizeof(size_t));
        	void* var5 = heap.stg_malloc(sizeof(size_t));
	        void* var6 = heap.stg_malloc(sizeof(size_t));
        	heap.stg_free(var5);
	        heap.stg_free(var6);
	}
	/*TEST(PageTreatments)
	{
	}
	TEST(LocalTreatments)
	{
	}
	TEST(LargeTreatments)
	{
	}
	TEST(FrameworkTests)
	{
	}*/
}
