#pragma once

#include <cstddef>

#define DEBUG_COHERENCY_CHECK
#define DEBUG_ZERO_OLD_MEM

#ifdef DEBUG_COHERENCY_CHECK
/*If true, will print the error and then exit(-1);*/
#define CHECK(COND, MESG) if(COND) \
{\
	fprintf(stderr, MESG);\
    exit(-1);\
}
#else
#define CHECK(COND, MESG) //Does nothing, to change this, enable the "COHERENCY_CHECK_DEBUG" define in Debug.hpp.
#endif

#ifdef DEBUG_ZERO_OLD_MEM
//maybe this hsould use std::memset?
//https://stackoverflow.com/questions/47381520/how-to-get-efficient-asm-for-zeroing-a-tiny-struct-with-msvc-for-x86-32?rq=3
#define BULK_ZERO(ADDR, LEN) \
{\
	unsigned long long len = LEN;\
	char* addr = ((char*)(ADDR));\
	CHECK(*((long long*)&len) < 0, "Trying to bulk zero, but LEN is likely negative!");\
	char rem = len % sizeof(unsigned long long*); /*unwound to multiples of 8*/\
	for (size_t i = 0; i < len / sizeof(unsigned long long*); i++)\
		*(((unsigned long long*)addr) + i) = 0ULL;\
	for (size_t i = len - rem; i < len; i++)\
		*(addr + i) = 0;\
}
#else
#define BULK_ZERO(ADDR, LEN) //Does nothing, to change this, enable the "DEBUG_ZERO_OLD_MEM" define in Debug.hpp.
#endif
