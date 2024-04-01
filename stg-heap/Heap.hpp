#pragma once

#include "HeapStructures.hpp"

//parameterizing at compile time seems too hard for now
//do not modify this
//#define MINIMUM_ALIGNMENT 2
//you may modify this
//#define DESIRED_ALIGNMENT 16
//TODO: math this
//#define ALIGNMENT 69420

class STGHeap;

class STGHeap
{

	//TODO: Heap visualizer for debugging

	//TODO: this variable needs to be initialized to an actual page(s), not just a global variable.
	//a pageheader* should reside in dynamic page(s), not in .bss, probably need an init() function to create
	//a single page and have this point to it.
	/////////////////PageHeader* pagesHandle; //global reference
	//might be good for the pages in the pages dll to be sorted big (prev) to small (next)?
	//might be good for there to be two global page pointers (head/tail)
	//the pagelist could be sorted "big" to "small", where big/small in the context of pages is determined by the # of pages that a particular PageHeader represents.
	//e.g., if a pageheader has a size of 500 (pages), then it would be "big", but if it's only 2, then it would be "small".
private:
	PageHeader *pd_head, *pd_tail; //"head" is large, "tail" is small
public:
	STGHeap(const STGHeap& a) = delete;
	STGHeap();
	~STGHeap();
	void* stg_malloc(size_t size);
	void stg_free(void* p);
private:
	//TODO: function for creating new pages and sink/swim them trhough the page directory dll.
	PageHeader* create_and_init_page(size_t minSizeBytes, bool overheadAlreadyIncluded);
	void heap_check();
	//friend class HeapTests;
};