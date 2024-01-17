#include "SELib/Threading.h"

void threading::Sleep(int ms)
{
    gBS->Stall(ms);
}
