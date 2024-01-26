#pragma once

#include <SELib/ia32.h>
#include <SELib/Memory.h>
#include <Arch/Pte.h>

namespace identity {
	typedef class _IDENTITY_MAPPING {
	public:
		__declspec(align(PAGE_SIZE)) PML4E_64 pml4[512];
		__declspec(align(PAGE_SIZE)) PDPTE_64 pdpt[512];
		__declspec(align(PAGE_SIZE)) PDE_2MB_64 pdt[512][512];

		UINT64 pa;

		void Init();
	} IDENTITY_MAPPING, * PIDENTITY_MAPPING;

	typedef union _VIRT_ADD
	{
		UINT64 value;
		struct
		{
			UINT64 offset_4kb : 12;
			UINT64 pt_index : 9;
			UINT64 pdt_index : 9;
			UINT64 pdpt_index : 9;
			UINT64 pml4_index : 9;
			UINT64 reserved : 16;
		};

		struct
		{
			UINT64 offset_2mb : 21;
			UINT64 pdt_index : 9;
			UINT64 pdpt_index : 9;
			UINT64 pml4_index : 9;
			UINT64 reserved : 16;
		};

		struct
		{
			UINT64 offset_1gb : 30;
			UINT64 pdpt_index : 9;
			UINT64 pml4_index : 9;
			UINT64 reserved : 16;
		};

	} VIRT_ADD, * PVIRT_ADD;

	void Init();
	void Dispose();

	UINT64 IdentityPa();

	PVOID MapIdentity(CR3 cr3);
	PVOID MapIdentityUntracked(CR3 cr3);

	UINT64 LastMappedPml4Index();
	CR3 LastMappedCR3();

	void ResetCache(bool bVmxRoot = false);
	void ResetCacheUntracked(CR3 cr3);

	class PhysicalAccess {
	private:
		char* pIdentity;
		CR3 cr3;
		bool bAllocated;

		template <typename T>
		typename T::Layout* phys2virt(const T phys)
		{
			UINT64 nig = *(UINT64*)&phys;
			return  (typename T::Layout*)(((UINT64)(pIdentity)+nig));
		}
#define IS_PT_BAD(pt) ((!pt->layout.P || ! pt->layout.A || !pt->layout.US))

	public:
		PhysicalAccess();
		PhysicalAccess(UINT64 cr3);
		PhysicalAccess(PVOID identity, UINT64 cr3);
		~PhysicalAccess();

		void SetCR3(UINT64 cr3) {
			this->cr3.Flags = cr3;
		}

		void* phys2virt(UINT64 phys)
		{
			return (void*)(pIdentity + phys);
		}

		unsigned long long getPhysicalAddress(UINT64 virtualAddress)
		{
			__try {
				constexpr auto k_mode = Pte::Mode::longMode4Level;

				using LinearAddress = Pte::LinearAddress<k_mode>;
				using Tables = Pte::Tables<k_mode>;

				LinearAddress addr{ 0 };
				addr.raw = virtualAddress;
				const auto cr3Pfn = this->cr3.AddressOfPageDirectory;

				const auto* const pml4e = phys2virt(Tables::pml4e(cr3Pfn, addr));

				//P is for Present, not 'P YOU SICK FUCKS
				if (!pml4e->layout.P)
				{
					return 0;
				}

				auto physPdpe = pml4e->pdpe(addr);
				VIRT_ADD_MAP map = { 0 };
				map.Flags = physPdpe.physicalAddress;
				if (map.Level4 != 0
					|| map.SignExt != 0
					)
					return 0;

				const auto* const pdpe = phys2virt(physPdpe);
				map.Flags = pdpe->nonPse.pde(addr).physicalAddress;
				if (IS_PT_BAD(pdpe) || pdpe->nonPse.layout.PD == 0)
					return 0;
				if (map.Level4 != 0
					|| map.SignExt != 0
					)
					return 0;

				switch (pdpe->pageSize())
				{
				case Pte::PageSize::nonPse:
				{
					const auto* const pde = phys2virt(pdpe->nonPse.pde(addr));
					map.Flags = pde->nonPse.pte(addr).physicalAddress;
					if (IS_PT_BAD(pde))
						return 0;

					if (map.Level4 != 0
						|| map.SignExt != 0
						)
						return 0;

					switch (pde->pageSize())
					{
					case Pte::PageSize::nonPse:
					{
						// 4Kb:
						const auto* const pte = phys2virt(pde->nonPse.pte(addr));
						if (!pte->page4Kb.P)
							return 0;
						const auto phys = pte->physicalAddress(addr);
						return phys.physicalAddress;
					}
					case Pte::PageSize::pse:
					{
						// 2Mb:
						const auto phys = pde->pse.physicalAddress(addr);
						if (!pde->pse.page2Mb.P) //?
							return 0;
						return phys.physicalAddress;
					}
					}
					break;
				}
				case Pte::PageSize::pse:
				{
					// 1Gb:
					const auto phys = pdpe->pse.physicalAddress(addr);
					if (!pdpe->pse.page1Gb.P) //?
						return 0;
					return phys.physicalAddress;
				}
				}
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {
				//KeBugCheckEx(0xaaaddd, virtualAddress, this->cr3.Flags, 0, 0);
			}

			return 0; // Invalid translation
		}

		template<typename T>
		T Read(UINT64 pa) {
			__try {
				return *(T*)(pIdentity + pa);
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {
				return T();
			}
		}
		template<typename T>
		void Write(UINT64 pa, T& obj) {
			__try {
				*(T*)(pIdentity + pa) = obj;
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {
			}
		}
		bool Read(PVOID src, PVOID dst, size_t size)
		{
			while (size) {
				UINT64 destSize = PAGE_SIZE - VIRT_ADD{ (UINT64)src }.offset_4kb;
				if (size < destSize)
					destSize = size;

				UINT64 srcSize = PAGE_SIZE - VIRT_ADD{ (UINT64)dst }.offset_4kb;
				if (size < srcSize)
					srcSize = size;
				UINT64 physAddr = 0;

				__try {
					physAddr = getPhysicalAddress((UINT64)src);

					if (!physAddr)
						return 0;

					UINT64 currentSize = min(destSize, srcSize);
					memory::memcpy(dst, phys2virt(physAddr), currentSize);

					dst = (PVOID)((UINT64)dst + currentSize);
					src = (PVOID)((UINT64)src + currentSize);
					size -= currentSize;
				}
				__except (EXCEPTION_EXECUTE_HANDLER) {
					return false;
				}
			}

			return true;
		}
		bool Write(PVOID dst, PVOID src, size_t size)
		{
			while (size) {
				UINT64 destSize = PAGE_SIZE - VIRT_ADD{ (UINT64)dst }.offset_4kb;
				if (size < destSize)
					destSize = size;

				UINT64 srcSize = PAGE_SIZE - VIRT_ADD{ (UINT64)src }.offset_4kb;
				if (size < srcSize)
					srcSize = size;
				__try {
					UINT64 physAddr = 0;
					physAddr = getPhysicalAddress((UINT64)dst);
					if (!physAddr)
						return 0;

					UINT64 currentSize = min(destSize, srcSize);
					memory::memcpy(phys2virt(physAddr), src, currentSize);

					src = (PVOID)((UINT64)src + currentSize);
					dst = (PVOID)((UINT64)dst + currentSize);
					size -= currentSize;
				}
				__except (EXCEPTION_EXECUTE_HANDLER) {
					return false;
				}
			}

			return true;
		}
		template<typename T>
		T Read(PVOID va) {
			T obj;
			Read(va, &obj, sizeof(obj));
			return obj;
		}
		template<typename T>
		void Write(PVOID va, T& obj) {
			Write(va, &obj, sizeof(obj));
		}

		template<typename T>
		void Write(UINT64 pa, T&& obj) {
			Write(pa, obj);
		}
		template<typename T>
		void Write(PVOID va, T&& obj) {
			Write(va, obj);
		}

		PVOID VirtToIdentityVirt(size_t va) {
			return (PVOID)phys2virt(getPhysicalAddress(va));
		}
	};
}