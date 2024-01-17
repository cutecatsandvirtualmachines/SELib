#include "MemoryEx.h"

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
