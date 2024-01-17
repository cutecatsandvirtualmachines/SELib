#include "SELib/Memory.h"

void* memory::eMalloc(size_t sz)
{
	VOID* PayLoadBuffer = NULL;
	gBS->AllocatePool(EfiBootServicesData, sz, &PayLoadBuffer);
	return PayLoadBuffer;
}

void memory::eFree(void* p)
{
	gBS->FreePool(p);
}

void memory::memset(void* dst, char val, size_t sz) {
	char* _dst = (char*)dst;
	for (size_t i = 0; i < sz; i++) {
		_dst[i] = val;
	}
}
