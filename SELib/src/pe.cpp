#include "SELib/pe.h"

#include <intrin.h>

PVOID pe::FindPE()
{
    SHORT* p = (SHORT*)PAGE_ALIGN(pe::FindPE);
    while (*p != 0x5a4d) {
        p -= 0x800;
    }
    return p;
}
