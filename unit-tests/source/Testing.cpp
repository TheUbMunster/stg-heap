#include "Heap.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <cstddef>
#include "gtest/gtest.h"
#include "Memory.hpp"

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
	TEST(SimpleTests, ExactPageSize) //requires manual observation to fully ensure correct behavior
	{
		STGHeap heap{};
		size_t size = p_size() - (sizeof(PageHeader) + sizeof(HeapNodeHeader) - sizeof(HeapNodeHeader::HeapNodeMandatoryBody) + sizeof(HeapNodeFooter));
		size = size - (size % 16);
		void* big = heap.stg_malloc(size);
	}
	TEST(SimpleTests, SlightlyTooBigPageSize) //requires manual observation to fully ensure correct behavior
	{
		STGHeap heap{};
		size_t size = p_size() - (sizeof(PageHeader) + sizeof(HeapNodeHeader) - sizeof(HeapNodeHeader::HeapNodeMandatoryBody) + sizeof(HeapNodeFooter));
		size = size - (size % 16);
		size++;
		void* toobig = heap.stg_malloc(size);
	}
	TEST(SimpleTests, MultiplePageSizesForceSwim) //requires manual observation to fully ensure correct behavior
	{
		STGHeap heap{};
		size_t multiplier = 4096;
		void* ptr1 = heap.stg_malloc(1 * multiplier); //will end up being pd_tail
		void* ptr2 = heap.stg_malloc(5 * multiplier); //will end up being pd_head
		void* ptr3 = heap.stg_malloc(2 * multiplier); //will end up swimming to the middle from pd_tail
	}
	TEST(SimpleTests, MultiplePageSizesForceSink) //requires manual observation to fully ensure correct behavior
	{
		STGHeap heap{};
		size_t multiplier = 4096;
		void* ptr1 = heap.stg_malloc(1 * multiplier); //will end up being pd_tail
		void* ptr2 = heap.stg_malloc(5 * multiplier); //will end up being pd_head
		void* ptr3 = heap.stg_malloc(4 * multiplier); //will end up sinking to the middle from pd_head
	}
	TEST(SimpleTest, AllocAndFreeWholePage) //requires manual observation to fully ensure correct behavior
	{
		STGHeap heap{};
		int* ptr1 = (int*)heap.stg_malloc(sizeof(int));
		int* ptr2 = (int*)heap.stg_malloc(sizeof(int));
		heap.stg_free(ptr1);
		heap.stg_free(ptr2);
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
