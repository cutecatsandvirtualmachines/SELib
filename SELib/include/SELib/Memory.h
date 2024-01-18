#pragma once

#include <SELib/Globals.h>

#define PAGE_SIZE 0x1000
#define LARGE_PAGE_SIZE (PAGE_SIZE * 512)
#define OFFSET_2MB(x) (x & (LARGE_PAGE_SIZE - 1))

namespace memory {
	void* eMalloc(size_t sz);
	void eFree(void* p);

	void memset(void* dst, char val, size_t sz);
}