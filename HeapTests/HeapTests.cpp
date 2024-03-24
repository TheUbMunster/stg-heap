#include "pch.h"
#include "CppUnitTest.h"
#include "Heap.hpp"
#include "Memory.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace HeapTests
{
	TEST_CLASS(HeapTests)
	{
	public:
		TEST_METHOD(ConstructDestruct)
		{
			STGHeap heap{};
		}
		TEST_METHOD(SimpleAlloc)
		{
			STGHeap heap{};
			int* ptr = (int*)heap.stg_malloc(sizeof(int));
			Assert::IsNotNull(ptr);
			*ptr = 5;
			Assert::AreEqual(5, *ptr);
		}
		TEST_METHOD(TinyAlloc)
		{
			STGHeap heap{};
			void* ptr = heap.stg_malloc(sizeof(char));
			Assert::IsNotNull(ptr);
		}
		TEST_METHOD(ExactAlloc)
		{
			STGHeap heap{};
			void* ptr = heap.stg_malloc(16);
			Assert::IsNotNull(ptr);
		}
		TEST_METHOD(WholePageAlloc)
		{
			STGHeap heap{};
			size_t size = p_size() - (sizeof(PageHeader) + sizeof(HeapNodeHeader) - sizeof(HeapNodeHeader::HeapNodeMandatoryBody) + sizeof(HeapNodeFooter)); //largest free literal space.
			size = size - (size % 16); //make sure that alignment is preserved.
			void* ptr = heap.stg_malloc(size);
			Assert::IsNotNull(ptr);
		}
		TEST_METHOD(SlightlyTooBigAlloc)
		{
			STGHeap heap{};
			size_t size = p_size() - (sizeof(PageHeader) + sizeof(HeapNodeHeader) - sizeof(HeapNodeHeader::HeapNodeMandatoryBody) + sizeof(HeapNodeFooter)); //largest free literal space.
			size = size - (size % 16); //make sure that alignment is preserved.
			void* ptr = heap.stg_malloc(size + 1);
			Assert::IsNotNull(ptr);
		}
	};
}
