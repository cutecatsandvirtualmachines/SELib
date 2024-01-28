#include "SELib/Vmcall.h"

__declspec(dllexport) UINT64 vmcallKey = 0;

bool vmcall::IsVmcall(UINT64 r9)
{
    return r9 == (vmcallKey ^ 0xbabab00e);
}

bool vmcall::IsValidKey(UINT64 _key)
{
    return !vmcallKey || vmcallKey == _key;
}

void vmcall::SetKey(UINT64 _key)
{
    vmcallKey = _key;
}

UINT64 vmcall::GetKey()
{
    return vmcallKey;
}
