#include "SELib/IO.h"

#include <SELib/Globals.h>

void io::vga::Clear()
{
	gST->ConOut->ClearScreen(gST->ConOut);
}

void io::vga::Output(wchar_t* str)
{
	gST->ConOut->OutputString(gST->ConOut, (CHAR16*)str);
}
