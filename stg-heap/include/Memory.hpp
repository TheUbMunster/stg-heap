#pragma once

#include <cstddef>

void* p_alloc(size_t pageCount);
void p_free(void* pp, size_t pageCount);
size_t p_size();
