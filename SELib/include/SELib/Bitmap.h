#pragma once

#include <SELib/BasicTypes.h>

namespace bitmap {
	typedef struct _LARGE_BITMAP {
		UINT64 low;
		UINT64 high;
	} LARGE_BITMAP, * PLARGE_BITMAP;

	VOID SetBit(void* va, UINT32 bit, BOOLEAN bSet);
	BOOLEAN GetBit(void* va, UINT32 bit);

	template<typename T>
	T bits(UINT64 value, UINT64 start, UINT64 end) {
		if (end <= start)
			return 0;
		UINT64 bitmask = 0;
		for (UINT32 i = 0; i < sizeof(bitmask) * 8; i++) {
			if (i <= end && i >= start)
				SetBit(&bitmask, i, true);
		}

		return (T)((value & bitmask) >> start);
	}
}