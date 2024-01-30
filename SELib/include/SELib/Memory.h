#pragma once

#include <SELib/ia32.h>

#define PAGE_SIZE 0x1000
#define LARGE_PAGE_SIZE (PAGE_SIZE * 512)
#define OFFSET_2MB(x) (x & (LARGE_PAGE_SIZE - 1))

#define RtlZeroMemory(p, len) memory::memset(p, 0, len)
#define RtlCopyMemory(dst, src, len) memory::memcpy(dst, src, len)

#define PAGE_ALIGN(x) ((UINT64)x & ~0xfffull)

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

    __forceinline void memset(void* dst, char val, size_t sz) {
        char* _dst = (char*)dst;
        for (size_t i = 0; i < sz; i++) {
            _dst[i] = val;
        }
    }

    __forceinline void* memcpy(void* dst, void* src, size_t sz)
    {
        size_t i;

        /*

            * If the buffers don't overlap, it doesn't matter what direction

            * we copy in. If they do, it does, so just assume they always do.

            * We don't concern ourselves with the possibility that the region

            * to copy might roll over across the top of memory, because it's

            * not going to happen.

            *

            * If the destination is above the source, we have to copy

            * back to front to avoid overwriting the data we want to

            * copy.

            *

            *      dest:       dddddddd

            *      src:    ssssssss   ^

            *              |   ^  |___|

            *              |___|

            *

            * If the destination is below the source, we have to copy

            * front to back.

            *

            *      dest:   dddddddd

            *      src:    ^   ssssssss

            *              |___|  ^   |

            *                     |___|

            */

        if ((UINT64)dst < (UINT64)src) {

            /*

                * As author/maintainer of libc, take advantage of the

                * fact that we know memcpy copies forwards.

                */

            return memcpy(dst, src, sz);

        }

        /*

            * Copy by words in the common case. Look in memcpy.c for more

            * information.

            */

        if ((UINT64)dst % sizeof(long) == 0 &&

            (UINT64)src % sizeof(long) == 0 &&

            sz % sizeof(long) == 0) {

            long* d = (long*)dst;

            const long* s = (const long*)src;

            /*

                * The reason we copy index i-1 and test i>0 is that

                * i is unsigned -- so testing i>=0 doesn't work.

                */

            for (i = sz / sizeof(long); i > 0; i--) {

                d[i - 1] = s[i - 1];

            }

        }

        else {

            char* d = (char*)dst;

            const char* s = (const char*)src;

            for (i = sz; i > 0; i--) {

                d[i - 1] = s[i - 1];

            }

        }

        return dst;
    }
}