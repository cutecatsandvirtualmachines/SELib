#pragma once

#include <Globals.h>

namespace memory {
	void* eMalloc(size_t sz);
	void eFree(void* p);
}