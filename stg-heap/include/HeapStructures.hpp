#pragma once

#include <cstddef>

struct HeapNodeHeader;
struct HeapNodeFooter;
struct PageHeader;

struct HeapNodeHeader
{
	HeapNodeHeader();
	HeapNodeHeader(unsigned long long size, bool isCurrentFree);
	//packed:
	//is current free flag  //1b
	//current size          //8B-3b since the size is a multiple of 8 bytes.
	
	//I think the size refers to the size of the body of the node (not including header/footer size).
	//it doesn't refer to the requested size for malloc, it refers to the actual size that malloc chose.
	unsigned long long header_data;
	union HeapNodeMandatoryBody //maybe separate this out so that sizeof(HeapNodeHeader) doesn't implicitly include this size, not intuitive.
	{
		void* mem;
		struct
		{
			//"prev" goes larger, "next" goes smaller
			HeapNodeHeader *prev, *next;
		} efl;
	} content;
	bool isImmediatePreviousFree();
	bool isCurrentFree();
	void setCurrentFree(bool isFree);
	bool isImmediateNextFree();
	unsigned long long size(); //size of the ACTUALLY ALLOCATED region (just the body, not including the header or the footer).
	void setSize(unsigned long long size);
	HeapNodeHeader* immediatePrevNeighbor();
	HeapNodeFooter* myFooter();
	HeapNodeHeader* immediateNextNeighbor();
};

struct HeapNodeFooter
{
	HeapNodeHeader* header; //technechally this pointer is 16-byte aligned, which means we would have room to store flags in the bottom bits if we wanted to (the hassle doesn't seem worth it though).
};

struct PageHeader
{
	//"prev" goes larger, "next" goes smaller
	PageHeader *prev, *next; //linked list of pages.
	//TODO: add other prev/next pointers that skip multiple pages, logarithmic. (e.g., a next that skips one page, one that skips three pages, one that skips seven pages, etc.)
	HeapNodeHeader *efl_head, *efl_tail; //head is large things, tail is small
	unsigned long long sizeBytes; //the size of these page(s) in bytes, multiple of p_size(), this includes the size of the page header itself.
	HeapNodeHeader* immediateFirstNode(); //very first element in this page(s) block
	HeapNodeHeader* immediateLastNode(); //very last element in this page(s) block
};

//constexpr size_t mandatoryHeapNodeMinSize = sizeof(HeapNodeHeader) + sizeof(HeapNodeFooter); //heapNodeHeader implicitly contains room for EFL.
