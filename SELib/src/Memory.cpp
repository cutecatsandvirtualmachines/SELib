#include "SELib/Memory.h"

#include <SELib/Globals.h>

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

void* memory::memcpy(void* dst, void* src, size_t sz)
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
