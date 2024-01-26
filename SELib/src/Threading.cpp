#include "SELib/Threading.h"

#include <SELib/Globals.h>

void threading::Sleep(int ms)
{
    gBS->Stall(ms);
}
