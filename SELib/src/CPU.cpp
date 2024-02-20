#include <SELib/CPU.h>

#include "Arch/Cpuid.h"

bool CPU::bIntelCPU = false;

void CPU::Init() {
    bIntelCPU = IsIntelCPU();
}

bool CPU::IsIntelCPU()
{
    bool isIntel = Cpuid::Cpuid::query<Cpuid::Generic::MaximumFunctionNumberAndVendorId>()->isIntel();
    return isIntel;
}