#include "Memory.hpp"
//fprintf and exit
#include <stdio.h>
#include <stdlib.h>
#include <cstddef>

#if _WIN32
//#include <memoryapi.h>
//#include <errhandlingapi.h>
//#include <sysinfoapi.h>
#include <Windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#endif

//TODO: if CLEANUP_DEBUG, then ensure the p_alloc'd pages are all zeroed out (they should be by default, but double check this is the case).
void* p_alloc(size_t pageCount)
{
	void* result;
	size_t byteCount = pageCount * p_size();
	//both VirtualAlloc and mmap want size in bytes?
#if _WIN32
	//https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc
	result = VirtualAlloc(NULL, byteCount, MEM_COMMIT, PAGE_READWRITE);
	if (result == NULL)
	{
		//panic
		fprintf(stderr, "Windows Virtual Alloc Failed: %d", GetLastError());
		exit(-1);
	}
#else
	result = mmap(0, byteCount, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
	if (result == MAP_FAILED)
	{
		fprintf(stderr, "POSIX mmap failed: %d", errno);
		exit(-1);
	}
#endif
	return result;
}
/// <summary>
/// When freeing pages that were allocated by p_alloc, you must free *all* of the pages for a given p_alloc-ation.
/// </summary>
void p_free(void* pp, size_t pageCount)
{
	size_t byteCount = pageCount * p_size();
#if _WIN32
	if (!VirtualFree(pp, 0, MEM_RELEASE)) //after "DECOMMIT", pages are in the "RESERVED" state. These 
												   //"RESERVED" pages can be reallocated without any further intervention via VirtualAlloc
	{
		//panic
		fprintf(stderr, "Windows Virtual Free Failed: %d", GetLastError());
		exit(-1);
	}
#else
	if (munmap(pp, byteCount))
	{
		fprintf(stderr, "POSIX munmap failed: %d", errno);
		exit(-1);
	}
#endif
}
size_t p_size()
{
	static size_t ps = 0;
#if _WIN32
	if (ps == 0)
	{
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		ps = si.dwPageSize;
	}
#else
	if (ps == 0)
		ps = getpagesize();
#endif
	return ps;
}
