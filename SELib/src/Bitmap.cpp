#include "SELib/Bitmap.h"

UINT64 Power2(UINT64 pw) {
	return 1ull << pw;
}

void bitmap::SetBit(void* va, UINT32 bit, BOOLEAN bSet)
{
	UINT32 byte = bit / 8;
	bit %= 8;

	if (bSet) {
		((char*)va)[byte] |= Power2(bit);
	}
	else {
		((char*)va)[byte] &= ~Power2(bit);
	}
}

BOOLEAN bitmap::GetBit(void* va, UINT32 bit)
{
	unsigned char byte = ((char*)va)[bit / 8];
	bit %= 8;

	return byte & Power2(bit);
}