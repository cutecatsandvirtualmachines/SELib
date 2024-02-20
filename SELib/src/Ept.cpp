#include "SELib/Ept.h"

VMM_EPT_PAGE_TABLE slat = { 0 };

BOOLEAN BuildMtrrMap(EPT_STATE* pEptState)
{
    IA32_MTRR_CAPABILITIES_REGISTER MTRRCap;
    IA32_MTRR_PHYSBASE_REGISTER CurrentPhysBase;
    IA32_MTRR_PHYSMASK_REGISTER CurrentPhysMask;
    PMTRR_RANGE_DESCRIPTOR Descriptor;
    UINT32 CurrentRegister;
    UINT32 NumberOfBitsInMask;

    MTRRCap.Flags = __readmsr(MSR_IA32_MTRR_CAPABILITIES);

    for (CurrentRegister = 0; CurrentRegister < MTRRCap.VariableRangeCount; CurrentRegister++)
    {
        // For each dynamic register pair
        CurrentPhysBase.Flags = __readmsr(MSR_IA32_MTRR_PHYSBASE0 + (CurrentRegister * 2));
        CurrentPhysMask.Flags = __readmsr(MSR_IA32_MTRR_PHYSMASK0 + (CurrentRegister * 2));

        // Is the range enabled?
        if (CurrentPhysMask.Valid)
        {
            // We only need to read these once because the ISA dictates that MTRRs are to be synchronized between all processors
            // during BIOS initialization.
            Descriptor = &pEptState->MemoryRanges[pEptState->NumberOfEnabledMemoryRanges++];

            // Calculate the base address in bytes
            Descriptor->PhysicalBaseAddress = CurrentPhysBase.PageFrameNumber * PAGE_SIZE;

            // Calculate the total size of the range
            // The lowest bit of the mask that is set to 1 specifies the size of the range
            _BitScanForward64((unsigned long*) & NumberOfBitsInMask, CurrentPhysMask.PageFrameNumber * PAGE_SIZE);

            // Size of the range in bytes + Base Address
            Descriptor->PhysicalEndAddress = Descriptor->PhysicalBaseAddress + ((1ULL << NumberOfBitsInMask) - 1ULL);

            // Memory Type (cacheability attributes)
            Descriptor->MemoryType = (UINT8)CurrentPhysBase.Type;

            if (Descriptor->MemoryType == MEMORY_TYPE_WRITE_BACK)
            {
                /* This is already our default, so no need to store this range.
                 * Simply 'free' the range we just wrote. */
                pEptState->NumberOfEnabledMemoryRanges--;
            }
        }
    }

    return true;
}

VOID SetupPML2Entry(EPT_STATE* pEptState, PPML2E_2MB NewEntry, UINT64 PageFrameNumber)
{
    UINT64 AddressOfPage;
    UINT64 CurrentMtrrRange;
    UINT64 TargetMemoryType;

    /*
      Each of the 512 collections of 512 PML2 entries is setup here.
      This will, in total, identity map every physical address from 0x0 to physical address 0x8000000000 (512GB of memory)

      ((EntryGroupIndex * VMM_EPT_PML2E_COUNT) + EntryIndex) * 2MB is the actual physical address we're mapping
     */
    NewEntry->SetPFN(PageFrameNumber);
    NewEntry->SetValid(true);
    NewEntry->SetReadWrite(true);
    NewEntry->SetUser(true);

    // Size of 2MB page * PageFrameNumber == AddressOfPage (physical memory). 
    AddressOfPage = PageFrameNumber * SIZE_2_MB;

    /* To be safe, we will map the first page as UC as to not bring up any kind of undefined behavior from the
      fixed MTRR section which we are not formally recognizing (typically there is MMIO memory in the first MB).

      I suggest reading up on the fixed MTRR section of the manual to see why the first entry is likely going to need to be UC.
     */
    if (PageFrameNumber == 0)
    {
        NewEntry->SetPATWriteback(false);
        return;
    }

    // Default memory type is always WB for performance. 
    TargetMemoryType = MEMORY_TYPE_WRITE_BACK;

    // For each MTRR range 
    for (CurrentMtrrRange = 0; CurrentMtrrRange < pEptState->NumberOfEnabledMemoryRanges; CurrentMtrrRange++)
    {
        // If this page's address is below or equal to the max physical address of the range 
        // And this page's last address is above or equal to the base physical address of the range 
        if (AddressOfPage <= pEptState->MemoryRanges[CurrentMtrrRange].PhysicalEndAddress
            && (AddressOfPage + SIZE_2_MB - 1) >= pEptState->MemoryRanges[CurrentMtrrRange].PhysicalBaseAddress)
        {
            /* If we're here, this page fell within one of the ranges specified by the variable MTRRs
               Therefore, we must mark this page as the same cache type exposed by the MTRR
            */
            TargetMemoryType = pEptState->MemoryRanges[CurrentMtrrRange].MemoryType;

            // 11.11.4.1 MTRR Precedences 
            if (TargetMemoryType == MEMORY_TYPE_UNCACHEABLE)
            {
                // If this is going to be marked uncacheable, then we stop the search as UC always takes precedent. 
                break;
            }
        }
    }

    // Finally, commit the memory type to the entry. 
    NewEntry->SetPATWriteback(TargetMemoryType == MEMORY_TYPE_WRITE_BACK);
}

EPT_STATE ept::Init(fnVirtToPhy pVirtToPhy)
{
    EPT_STATE eptState = { 0 };
    PVMM_EPT_PAGE_TABLE PageTable;
    PML3E RWXTemplate;
    UINT64 EntryGroupIndex;
    UINT64 EntryIndex;

    PageTable = (PVMM_EPT_PAGE_TABLE)&slat;

    BuildMtrrMap(&eptState);

    // Zero out all entries to ensure all unused entries are marked Not Present 
    RtlZeroMemory(PageTable, (UINT64)sizeof(VMM_EPT_PAGE_TABLE));

    // Mark the first 512GB PML4 entry as present, which allows us to manage up to 512GB of discrete paging structures. 
    PageTable->PML4[0].SetPFN((UINT64)pVirtToPhy((UINT64) & PageTable->PML3[0]) / PAGE_SIZE);
    PageTable->PML4[0].SetReadWrite(true);
    PageTable->PML4[0].SetExecute(true);
    PageTable->PML4[0].SetValid(true);
    PageTable->PML4[0].SetUser(true);

    /* Now mark each 1GB PML3 entry as RWX and map each to their PML2 entry */

    // Ensure stack memory is cleared
    RWXTemplate.Flags = 0;

    // Set up one 'template' RWX PML3 entry and copy it into each of the 512 PML3 entries 
    // Using the same method as SimpleVisor for copying each entry using intrinsics. 
    RWXTemplate.SetReadWrite(true);
    RWXTemplate.SetExecute(true);
    RWXTemplate.SetValid(true);
    RWXTemplate.SetUser(true);

    // Copy the template into each of the 512 PML3 entry slots 
    __stosq((UINT64*)&PageTable->PML3[0], RWXTemplate.Flags, VMM_EPT_PML3E_COUNT);

    // For each of the 512 PML3 entries 
    for (EntryIndex = 0; EntryIndex < VMM_EPT_PML3E_COUNT; EntryIndex++)
    {
        // Map the 1GB PML3 entry to 512 PML2 (2MB) entries to describe each large page.
        // NOTE: We do *not* manage any PML1 (4096 byte) entries and do not allocate them.
        PageTable->PML3[EntryIndex].SetPFN((UINT64)pVirtToPhy((UINT64) & PageTable->PML2[EntryIndex][0]) / PAGE_SIZE);
    }

    PML2E_2MB PML2EntryTemplate = { 0 };
    PML2EntryTemplate.SetExecute(true);

    PML2EntryTemplate.SetLarge(true);
    PML2EntryTemplate.SetValid(true);
    PML2EntryTemplate.SetReadWrite(true);

    PML2EntryTemplate.SetUser(true);
    PML2EntryTemplate.amd.Accessed = true;
    PML2EntryTemplate.amd.Dirty = true;
    /* For each collection of 512 PML2 entries (512 collections * 512 entries per collection), mark it RWX using the same template above.
       This marks the entries as "Present" regardless of if the actual system has memory at this region or not. We will cause a fault in our
       EPT handler if the guest access a page outside a usable range, despite the EPT frame being present here.
     */
    __stosq((UINT64*)&PageTable->PML2[0], PML2EntryTemplate.Flags, VMM_EPT_PML3E_COUNT * VMM_EPT_PML2E_COUNT);

    // For each of the 512 collections of 512 2MB PML2 entries 
    for (EntryGroupIndex = 0; EntryGroupIndex < VMM_EPT_PML3E_COUNT; EntryGroupIndex++)
    {
        // For each 2MB PML2 entry in the collection 
        for (EntryIndex = 0; EntryIndex < VMM_EPT_PML2E_COUNT; EntryIndex++)
        {
            // Setup the memory type and frame number of the PML2 entry. 
            auto ppml2 = (PPML2E_2MB)&PageTable->PML2[EntryGroupIndex][EntryIndex];
            SetupPML2Entry(&eptState, ppml2, (EntryGroupIndex * VMM_EPT_PML2E_COUNT) + EntryIndex);
        }
    }

    return eptState;
}
