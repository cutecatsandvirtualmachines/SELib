#pragma once

#include <SELib/ia32.h>

#define PAGE_SIZE 0x1000
#define LARGE_PAGE_SIZE (PAGE_SIZE * 512)
#define OFFSET_2MB(x) (x & (LARGE_PAGE_SIZE - 1))

#define RtlZeroMemory(p, len) memory::memset(p, 0, len)
#define RtlCopyMemory(dst, src, len) memory::memcpy(dst, src, len)

typedef struct _PML4T {
    PML4E_64 entry[512];
} PML4T, * PPML4T;

typedef struct _PDPT {
    PDPTE_64 entry[512];
} PDPT, * PPDPT;

typedef struct _PDT {
    PDE_64 entry[512];
} PDT, * PPDT;

typedef struct _PT {
    PTE_64 entry[512];
} PT, * PPT;

typedef union {
    struct {
        UINT64 Offset : 12;
        UINT64 Level1 : 9;
        UINT64 Level2 : 9;
        UINT64 Level3 : 9;
        UINT64 Level4 : 9;
        UINT64 SignExt : 16;
    };

    UINT64 Flags;
} VIRT_ADD_MAP, * PVIRT_ADD_MAP;

namespace memory {
	void* eMalloc(size_t sz);
	void eFree(void* p);

	void memset(void* dst, char val, size_t sz);
    void* memcpy(void* dst, void* src, size_t sz);
}