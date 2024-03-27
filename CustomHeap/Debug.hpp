#pragma once

#define COHERENCY_CHECK_DEBUG

#ifdef COHERENCY_CHECK_DEBUG
/*If true, will print the error and then exit(-1);*/
#define CHECK(COND, MESG) if(COND) \
{\
	fprintf(stderr, MESG);\
    exit(-1);\
}
#else
#define CHECK(COND, MESG) //Does nothing, to change this, enable the "COHERENCY_CHECK_DEBUG" define in Debug.hpp.
#endif