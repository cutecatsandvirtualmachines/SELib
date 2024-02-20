#pragma once

#include "SELib/BasicTypes.h"
#include "SELib/CPU.h"
#include "Memory.h"

// The number of 512GB PML4 entries in the page table/
#define VMM_EPT_PML4E_COUNT 512
// The number of 1GB PDPT entries in the page table per 512GB PML4 entry.
#define VMM_EPT_PML3E_COUNT 512
// Then number of 2MB Page Directory entries in the page table per 1GB PML3 entry.
#define VMM_EPT_PML2E_COUNT 512
// Then number of 4096 byte Page Table entries in the page table per 2MB PML2 entry when dynamically split.
#define VMM_EPT_PML1E_COUNT 512

typedef union _EPT_PML4E {
    UINT64 Flags;
    struct {
        UINT64 Read : 1; // bit 0
        UINT64 Write : 1; // bit 1
        UINT64 Execute : 1; // bit 2
        UINT64 Reserved1 : 5; // bit 7:3 (Must be Zero)
        UINT64 Accessed : 1; // bit 8
        UINT64 Ignored1 : 1; // bit 9
        UINT64 ExecuteForUserMode : 1; // bit 10
        UINT64 Ignored2 : 1; // bit 11
        UINT64 PhysicalAddress : 40; // bit (N-1):12 or Page-Frame-Number
        UINT64 Ignored3 : 12; // bit 63:52
    } intel;
    struct
    {
        UINT64 Valid : 1;               // [0]
        UINT64 Write : 1;               // [1]
        UINT64 User : 1;                // [2]
        UINT64 WriteThrough : 1;        // [3]
        UINT64 CacheDisable : 1;        // [4]
        UINT64 Accessed : 1;            // [5]
        UINT64 Ignored0 : 1;               // [6]
        UINT64 PageSize : 1;                 // [7]
        UINT64 Ignored1 : 1;              // [8]
        UINT64 Avl : 3;                 // [9:11]
        UINT64 PageFrameNumber : 40;    // [12:51]
        UINT64 Reserved1 : 11;          // [52:62]
        UINT64 NoExecute : 1;           // [63]
    } amd;
    __forceinline void SetValid(bool bSet = true) {
        if (!CPU::bIntelCPU) {
            amd.Valid = bSet;
            amd.User = bSet;
        }
    }
    __forceinline void SetExecute(bool bSet = true) {
        if (CPU::bIntelCPU)
            intel.Execute = bSet;
        else
            amd.NoExecute = !bSet;
    }
    __forceinline void SetReadWrite(bool bSet = true) {
        if (CPU::bIntelCPU) {
            intel.Write = bSet;
            intel.Read = bSet;
        }
        else {
            amd.Write = bSet;
        }
    }
    __forceinline void SetPFN(UINT64 pfn) {
        if (CPU::bIntelCPU) {
            intel.PhysicalAddress = pfn;
        }
        else {
            amd.PageFrameNumber = pfn;
        }
    }
    __forceinline UINT64 GetPFN() {
        if (CPU::bIntelCPU) {
            return intel.PhysicalAddress;
        }
        else {
            return amd.PageFrameNumber;
        }
    }
    __forceinline bool GetUser() {
        if (!CPU::bIntelCPU)
            return amd.User;
    }
    __forceinline void SetUser(bool bSet) {
        if (!CPU::bIntelCPU)
            amd.User = bSet;
    }
} EPT_PML4E, * PEPT_PML4E;

typedef union _EPT_PDPTE
{
    UINT64 Flags;
    struct
    {
        UINT64 ReadAccess : 1;
        UINT64 WriteAccess : 1;
        UINT64 ExecuteAccess : 1;
        UINT64 Reserved1 : 5;
        UINT64 Accessed : 1;
        UINT64 Reserved2 : 1;
        UINT64 UserModeExecute : 1;
        UINT64 Reserved3 : 1;
        UINT64 PageFrameNumber : 40;
        UINT64 Reserved4 : 12;
    } intel;
    struct
    {
        UINT64 Valid : 1;               // [0]
        UINT64 Write : 1;               // [1]
        UINT64 User : 1;                // [2]
        UINT64 WriteThrough : 1;        // [3]
        UINT64 CacheDisable : 1;        // [4]
        UINT64 Accessed : 1;            // [5]
        UINT64 Ignored0 : 1;               // [6]
        UINT64 PageSize : 1;                 // [7]
        UINT64 Ignored1 : 1;              // [8]
        UINT64 Avl : 3;                 // [9:11]
        UINT64 PageFrameNumber : 40;    // [12:51]
        UINT64 Reserved1 : 11;          // [52:62]
        UINT64 NoExecute : 1;           // [63]
    } amd;
    __forceinline void SetValid(bool bSet = true) {
        if (!CPU::bIntelCPU) {
            amd.Valid = bSet;
            amd.User = bSet;
        }
    }
    __forceinline void SetExecute(bool bSet = true) {
        if (CPU::bIntelCPU)
            intel.ExecuteAccess = bSet;
        else
            amd.NoExecute = !bSet;
    }
    __forceinline void SetReadWrite(bool bSet = true) {
        if (CPU::bIntelCPU) {
            intel.WriteAccess = bSet;
            intel.ReadAccess = bSet;
        }
        else {
            amd.Write = bSet;
        }
    }
    __forceinline void SetPFN(UINT64 pfn) {
        if (CPU::bIntelCPU) {
            intel.PageFrameNumber = pfn;
        }
        else {
            amd.PageFrameNumber = pfn;
        }
    }
    __forceinline UINT64 GetPFN() {
        if (CPU::bIntelCPU) {
            return intel.PageFrameNumber;
        }
        else {
            return amd.PageFrameNumber;
        }
    }
    __forceinline bool GetUser() {
        if (!CPU::bIntelCPU)
            return amd.User;
    }
    __forceinline void SetUser(bool bSet) {
        if (!CPU::bIntelCPU)
            amd.User = bSet;
    }
} EPT_PDPTE, * PEPT_PDPTE;

typedef union _EPT_PDE {
    UINT64 Flags;
    struct
    {
        UINT64 ReadAccess : 1;
        UINT64 WriteAccess : 1;
        UINT64 ExecuteAccess : 1;
        UINT64 Reserved1 : 5;
        UINT64 Accessed : 1;
        UINT64 Reserved2 : 1;
        UINT64 UserModeExecute : 1;
        UINT64 Reserved3 : 1;
        UINT64 PageFrameNumber : 40;
        UINT64 Reserved4 : 12;
    } intel;
    struct
    {
        UINT64 Valid : 1;               // [0]
        UINT64 Write : 1;               // [1]
        UINT64 User : 1;                // [2]
        UINT64 WriteThrough : 1;        // [3]
        UINT64 CacheDisable : 1;        // [4]
        UINT64 Accessed : 1;            // [5]
        UINT64 Ignored0 : 1;               // [6]
        UINT64 PageSize : 1;                 // [7]
        UINT64 Ignored1 : 1;              // [8]
        UINT64 Avl : 3;                 // [9:11]
        UINT64 PageFrameNumber : 40;    // [12:51]
        UINT64 Reserved1 : 11;          // [52:62]
        UINT64 NoExecute : 1;           // [63]
    } amd;

    __forceinline void SetValid(bool bSet = true) {
        if (!CPU::bIntelCPU) {
            amd.Valid = bSet;
            amd.User = bSet;
        }
    }

    __forceinline void SetExecute(bool bSet = true) {
        if (CPU::bIntelCPU)
            intel.ExecuteAccess = bSet;
        else
            amd.NoExecute = !bSet;
    }
    __forceinline void SetReadWrite(bool bSet = true) {
        if (CPU::bIntelCPU) {
            intel.WriteAccess = bSet;
            intel.ReadAccess = bSet;
        }
        else {
            amd.Write = bSet;
        }
    }
    __forceinline void SetPFN(UINT64 pfn) {
        if (CPU::bIntelCPU) {
            intel.PageFrameNumber = pfn;
        }
        else {
            amd.PageFrameNumber = pfn;
        }
    }
    __forceinline UINT64 GetPFN() {
        if (CPU::bIntelCPU) {
            return intel.PageFrameNumber;
        }
        else {
            return amd.PageFrameNumber;
        }
    }
    __forceinline bool GetExecute() {
        if (CPU::bIntelCPU)
            return intel.ExecuteAccess;
        else
            return !amd.NoExecute;
    }
    __forceinline bool GetUser() {
        if (!CPU::bIntelCPU)
            return amd.User;
    }
    __forceinline void SetUser(bool bSet) {
        if (!CPU::bIntelCPU)
            amd.User = bSet;
    }
} EPT_PDE, * PEPT_PDE;

typedef union _EPT_PTE {
    UINT64 Flags;
    struct {
        UINT64 Read : 1; // bit 0
        UINT64 Write : 1; // bit 1
        UINT64 Execute : 1; // bit 2
        UINT64 EPTMemoryType : 3; // bit 5:3 (EPT Memory type)
        UINT64 IgnorePAT : 1; // bit 6
        UINT64 Ignored1 : 1; // bit 7
        UINT64 AccessedFlag : 1; // bit 8   
        UINT64 DirtyFlag : 1; // bit 9
        UINT64 ExecuteForUserMode : 1; // bit 10
        UINT64 Ignored2 : 1; // bit 11
        UINT64 PhysicalAddress : 40; // bit (N-1):12 or Page-Frame-Number
        UINT64 Ignored3 : 11; // bit 62:52
        UINT64 SuppressVE : 1; // bit 63
    } intel;
    struct
    {
        UINT64 Valid : 1;               // [0]
        UINT64 Write : 1;               // [1]
        UINT64 User : 1;                // [2]
        UINT64 WriteThrough : 1;        // [3]
        UINT64 CacheDisable : 1;        // [4]
        UINT64 Accessed : 1;            // [5]
        UINT64 Dirty : 1;               // [6]
        UINT64 Pat : 1;                 // [7]
        UINT64 Global : 1;              // [8]
        UINT64 Avl : 3;                 // [9:11]
        UINT64 PageFrameNumber : 40;    // [12:51]
        UINT64 Reserved1 : 11;          // [52:62]
        UINT64 NoExecute : 1;           // [63]
    } amd;


    __forceinline void SetValid(bool bSet = true) {
        if (!CPU::bIntelCPU) {
            amd.Valid = bSet;
            amd.User = bSet;
        }
    }
    __forceinline void SetExecute(bool bSet = true) {
        if (CPU::bIntelCPU)
            intel.Execute = bSet;
        else
            amd.NoExecute = !bSet;
    }
    __forceinline void SetReadWrite(bool bSet = true) {
        if (CPU::bIntelCPU) {
            intel.Write = bSet;
            intel.Read = bSet;
        }
        else {
            amd.Write = bSet;
        }
    }
    __forceinline void SetPFN(UINT64 pfn) {
        if (CPU::bIntelCPU) {
            intel.PhysicalAddress = pfn;
        }
        else {
            amd.PageFrameNumber = pfn;
        }
    }
    __forceinline void SetPATWriteback(bool bSet = true) {
        if (CPU::bIntelCPU) {
            intel.EPTMemoryType = bSet ? MEMORY_TYPE_WRITE_BACK : MEMORY_TYPE_UNCACHEABLE;
        }
        else {
            amd.WriteThrough = false;
        }
    }
    __forceinline UINT64 GetPFN() {
        if (CPU::bIntelCPU) {
            return intel.PhysicalAddress;
        }
        else {
            return amd.PageFrameNumber;
        }
    }
    __forceinline bool GetExecute() {
        if (CPU::bIntelCPU)
            return intel.Execute;
        else
            return !amd.NoExecute;
    }
    __forceinline bool GetUser() {
        if (!CPU::bIntelCPU)
            return amd.User;
    }
    __forceinline void SetUser(bool bSet) {
        if (!CPU::bIntelCPU)
            amd.User = bSet;
    }
} EPT_PTE, * PEPT_PTE;

typedef union _EPT_PDE_2MB {
    UINT64 Flags;
    struct
    {
        UINT64 ReadAccess : 1;
        UINT64 WriteAccess : 1;
        UINT64 ExecuteAccess : 1;
        UINT64 MemoryType : 3;
        UINT64 IgnorePat : 1;
        UINT64 LargePage : 1;
        UINT64 Accessed : 1;
        UINT64 Dirty : 1;
        UINT64 UserModeExecute : 1;
        UINT64 Reserved1 : 10;
        UINT64 PageFrameNumber : 31;
        UINT64 Reserved2 : 11;
        UINT64 SuppressVe : 1;
    } intel;
    struct
    {
        UINT64 Valid : 1;               // [0]
        UINT64 Write : 1;               // [1]
        UINT64 User : 1;                // [2]
        UINT64 WriteThrough : 1;        // [3]
        UINT64 CacheDisable : 1;        // [4]
        UINT64 Accessed : 1;            // [5]
        UINT64 Dirty : 1;               // [6]
        UINT64 PageSize : 1;                 // [7]
        UINT64 Global : 1;              // [8]
        UINT64 Avl : 3;                 // [9:11]
        UINT64 PAT : 1; // Page-Attribute Table
        UINT64 : 8;
        UINT64 PageFrameNumber : 31;    // [12:51]
        UINT64 Reserved1 : 11;          // [52:62]
        UINT64 NoExecute : 1;           // [63]
    } amd;

    /*
                    unsigned long long P : 1; // Present
                    unsigned long long RW : 1; // Read/Write
                    unsigned long long US : 1; // User/Supervisor
                    unsigned long long PWT : 1; // Page-Level Writethrough
                    unsigned long long PCD : 1; // Page-Level Cache Disable
                    unsigned long long A : 1; // Accessed
                    unsigned long long D : 1; // Dirty
                    unsigned long long PS : 1; // PageSize == 1
                    unsigned long long G : 1; // Global Page
                    unsigned long long AVL : 3; // Available to software
                    unsigned long long PAT : 1; // Page-Attribute Table
                    unsigned long long : 8;
                    unsigned long long PhysicalPageFrameNumber : 31;
                    unsigned long long Available : 11;
                    unsigned long long NX : 1; // No Execute
*/
    __forceinline void SetValid(bool bSet = true) {
        if (!CPU::bIntelCPU) {
            amd.Valid = bSet;
            amd.User = bSet;
        }
    }
    __forceinline void SetExecute(bool bSet = true) {
        if (CPU::bIntelCPU)
            intel.ExecuteAccess = bSet;
        else
            amd.NoExecute = !bSet;
    }
    __forceinline void SetReadWrite(bool bSet = true) {
        if (CPU::bIntelCPU) {
            intel.WriteAccess = bSet;
            intel.ReadAccess = bSet;
        }
        else {
            amd.Write = bSet;
        }
    }
    __forceinline void SetPFN(UINT64 pfn) {
        if (CPU::bIntelCPU) {
            intel.PageFrameNumber = pfn;
        }
        else {
            amd.PageFrameNumber = pfn;
        }
    }
    __forceinline void SetPATWriteback(bool bSet = true) {
        if (CPU::bIntelCPU) {
            intel.MemoryType = bSet ? MEMORY_TYPE_WRITE_BACK : MEMORY_TYPE_UNCACHEABLE;
        }
        else {
            amd.WriteThrough = false;
        }
    }
    __forceinline void SetLarge(bool bSet = true) {
        if (CPU::bIntelCPU) {
            intel.LargePage = bSet;
        }
        else {
            amd.PageSize = bSet;
        }
    }
    __forceinline bool GetLarge() {
        if (CPU::bIntelCPU) {
            return intel.LargePage;
        }
        else {
            return amd.PageSize;
        }
    }
    __forceinline UINT64 GetPFN() {
        if (CPU::bIntelCPU) {
            return intel.PageFrameNumber;
        }
        else {
            return amd.PageFrameNumber;
        }
    }
    __forceinline bool GetExecute() {
        if (CPU::bIntelCPU)
            return intel.ExecuteAccess;
        else
            return !amd.NoExecute;
    }
    __forceinline bool GetUser() {
        if (!CPU::bIntelCPU)
            return amd.User;
    }
    __forceinline void SetUser(bool bSet) {
        if (!CPU::bIntelCPU)
            amd.User = bSet;
    }
} EPT_PDE_2MB, * PEPT_PDE_2MB;

typedef EPT_PML4E PML4E, * PPML4E;
typedef EPT_PDPTE PML3E, * PPML3E;
typedef EPT_PDE_2MB PML2E_2MB, * PPML2E_2MB;
typedef EPT_PDE PML2E, * PPML2E;
typedef EPT_PTE PML1E, * PPML1E;

typedef union _EPTP
{
    struct
    {
        /**
         * [Bits 2:0] EPT paging-structure memory type:
         * - 0 = Uncacheable (UC)
         * - 6 = Write-back (WB)
         * Other values are reserved.
         *
         * @see Vol3C[28.2.6(EPT and memory Typing)]
         */
        UINT64 MemoryType : 3;

        /**
         * [Bits 5:3] This value is 1 less than the EPT page-walk length.
         *
         * @see Vol3C[28.2.6(EPT and memory Typing)]
         */
        UINT64 PageWalkLength : 3;

        /**
         * [Bit 6] Setting this control to 1 enables accessed and dirty flags for EPT.
         *
         * @see Vol3C[28.2.4(Accessed and Dirty Flags for EPT)]
         */
        UINT64 EnableAccessAndDirtyFlags : 1;
        UINT64 Reserved1 : 5;

        /**
         * [Bits 47:12] Bits N-1:12 of the physical address of the 4-KByte aligned EPT PML4 table.
         */
        UINT64 PageFrameNumber : 36;
        UINT64 Reserved2 : 16;
    };

    UINT64 Flags;
} EPTP, * PEPTP;

typedef struct _VMM_EPT_PAGE_TABLE
{
    /**
     * 28.2.2 Describes 512 contiguous 512GB memory regions each with 512 1GB regions.
     */
    DECLSPEC_ALIGN(PAGE_SIZE) PML4E PML4[VMM_EPT_PML4E_COUNT];

    /**
     * Describes exactly 512 contiguous 1GB memory regions within a our singular 512GB PML4 region.
     */
    DECLSPEC_ALIGN(PAGE_SIZE) PML3E PML3[VMM_EPT_PML3E_COUNT];

    /**
     * For each 1GB PML3 entry, create 512 2MB entries to map identity.
     * NOTE: We are using 2MB pages as the smallest paging size in our map, so we do not manage individiual 4096 byte pages.
     * Therefore, we do not allocate any PML1 (4096 byte) paging structures.
     */
    DECLSPEC_ALIGN(PAGE_SIZE) PML2E PML2[VMM_EPT_PML3E_COUNT][VMM_EPT_PML2E_COUNT];

} VMM_EPT_PAGE_TABLE, * PVMM_EPT_PAGE_TABLE;

typedef struct _MTRR_RANGE_DESCRIPTOR
{
    UINT64 PhysicalBaseAddress;
    UINT64 PhysicalEndAddress;
    UINT8  MemoryType;
} MTRR_RANGE_DESCRIPTOR, * PMTRR_RANGE_DESCRIPTOR;

typedef struct _EPT_STATE
{
    MTRR_RANGE_DESCRIPTOR MemoryRanges[9];							// Physical memory ranges described by the BIOS in the MTRRs. Used to build the EPT identity mapping.
    UINT32 NumberOfEnabledMemoryRanges;								// Number of memory ranges specified in MemoryRanges
    EPTP   EptPointer;												// Extended-Page-Table Pointer
    CR3    nCR3;
} EPT_STATE, * PEPT_STATE;

extern VMM_EPT_PAGE_TABLE slat;

namespace ept {
    EPT_STATE Init(fnVirtToPhy pVirtToPhy);
}