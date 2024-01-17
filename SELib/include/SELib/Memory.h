#pragma once

#include <SELib/Globals.h>

namespace memory {
	void* eMalloc(size_t sz);
	void eFree(void* p);

	void memset(void* dst, char val, size_t sz);
}