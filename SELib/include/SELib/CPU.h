#pragma once

#include <SELib/BasicTypes.h>

namespace CPU {
    extern "C" UINT64 GetGdtBase();
    extern "C" UINT16  GetGdtLimit();
    extern "C" UINT64 GetIdtBase();
    extern "C" UINT16  GetIdtLimit();

    extern "C" UINT16  GetCs();
    extern "C" UINT16  GetDs();
    extern "C" UINT16  GetEs();
    extern "C" UINT16  GetFs();
    extern "C" UINT16  GetSs();
    extern "C" UINT16  GetGs();
    extern "C" UINT16  GetTr();
    extern "C" UINT16  GetLdtr();
    extern "C" UINT64 GetRflags();

    extern "C" void SetCs(UINT16);
    extern "C" void SetDs(UINT16);
    extern "C" void SetEs(UINT16);
    extern "C" void SetFs(UINT16);
    extern "C" void SetSs(UINT16);
    extern "C" void SetGs(UINT16);
    extern "C" void SetTr(UINT16);
    extern "C" void SetLdtr(UINT16);
    extern "C" void SetRflags(UINT64);

    extern "C" void GetGdt(void* pGdt);
    extern "C" void SetGdt(void* pGdt);
    extern "C" void GetIdt(void* pIdt);
    extern "C" void SetIdt(void* pIdt);
    extern "C" void SetIF(bool bSet);

    extern "C" UINT32 InveptContext(UINT32 Type, void* Descriptors);
    extern "C" UINT32 InvalidateVPID(UINT32 Type, void* Descriptors);
    extern "C" void ClearTLB();
    extern "C" void Jump(void* rip);
    extern "C" void ChangeRSP(size_t rsp);

    extern "C" bool IsVmxSupported();
    extern "C" bool IsVmxEnabled();
    extern "C" bool EnableVmx();
    extern "C" bool DisableVmx();
}