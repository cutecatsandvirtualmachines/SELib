#include "SELib/Vmcall.h"

UINT64 key = 0;

bool vmcall::IsVmcall(UINT64 r9)
{
    return r9 == (0xbabab00e);
}

bool vmcall::IsValidKey(UINT64 _key)
{
    return !key || key == _key;
}

void vmcall::SetKey(UINT64 _key)
{
    key = _key;
}

UINT64 vmcall::GetKey()
{
    return key;
}
