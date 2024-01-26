#include "SELib/Identity.h"

#include <SELib/Memory.h>
#include <intrin.h>

identity::IDENTITY_MAPPING* mapping = nullptr;

bool bIdentityCreated = false;
UINT64 lastMappedIndex = MAXULONG64;
CR3 lastMappedCR3 = { 0 };

void identity::Init()
{
	if (bIdentityCreated)
		return;

	mapping = (identity::IDENTITY_MAPPING*)memory::eMalloc(sizeof(*mapping));
	if (!mapping) {
		return;
	}
	RtlZeroMemory(mapping, sizeof(*mapping));

	mapping->pml4[0].Present = true;
	mapping->pml4[0].Write = true;
	mapping->pml4[0].Supervisor = true;
	mapping->pml4[0].PageFrameNumber = (UINT64)&mapping->pdpt[0] / PAGE_SIZE;

	for (UINT64 EntryIndex = 0; EntryIndex < 512; EntryIndex++)
	{
		mapping->pdpt[EntryIndex].Present = true;
		mapping->pdpt[EntryIndex].Write = true;
		mapping->pdpt[EntryIndex].Supervisor = true;
		mapping->pdpt[EntryIndex].PageFrameNumber = (UINT64)&mapping->pdt[EntryIndex][0] / PAGE_SIZE;
	}

	for (UINT64 EntryGroupIndex = 0; EntryGroupIndex < 512; EntryGroupIndex++)
	{
		for (UINT64 EntryIndex = 0; EntryIndex < 512; EntryIndex++)
		{
			mapping->pdt[EntryGroupIndex][EntryIndex].Present = true;
			mapping->pdt[EntryGroupIndex][EntryIndex].Write = true;
			mapping->pdt[EntryGroupIndex][EntryIndex].LargePage = true;
			mapping->pdt[EntryGroupIndex][EntryIndex].Supervisor = true;
			mapping->pdt[EntryGroupIndex][EntryIndex].PageFrameNumber = (EntryGroupIndex * 512) + EntryIndex;
		}
	}

	bIdentityCreated = true;
}

void identity::Dispose()
{

}

UINT64 identity::IdentityPa()
{
	return (UINT64)mapping;
}

PVOID identity::MapIdentity(CR3 cr3)
{
	if (!bIdentityCreated) {
		return (PVOID)MAXULONG64;
	}

	if (lastMappedCR3.Flags) {
		if (lastMappedCR3.Flags == cr3.Flags) {
			VIRT_ADD_MAP virtAddMap = { 0 };
			virtAddMap.Level4 = lastMappedIndex;

			PPML4T ppml4 = (PPML4T)(cr3.AddressOfPageDirectory * PAGE_SIZE);
			ppml4->entry[lastMappedIndex].Flags = mapping->pml4[0].Flags;
			return (PVOID)virtAddMap.Flags;
		}
		return (PVOID)MAXULONG64;
	}

	if (!cr3.AddressOfPageDirectory) {
		return (PVOID)MAXULONG64;
	}

	PPML4T pml4 = (PPML4T)(cr3.AddressOfPageDirectory * PAGE_SIZE);

	bool bEntryFound = false;
	int pml4Idx = 0x80;
	for (; pml4Idx < 255; pml4Idx++) {
		if (pml4->entry[pml4Idx].Flags == 0) {
			bEntryFound = true;
			break;
		}
	}
	if (!bEntryFound) {
		return (PVOID)MAXULONG64;
	}

	pml4->entry[pml4Idx].Flags = mapping->pml4[0].Flags;

	lastMappedIndex = pml4Idx;
	lastMappedCR3.Flags = cr3.Flags;

	VIRT_ADD_MAP virtAddMap = { 0 };
	virtAddMap.Level4 = pml4Idx;

	return (PVOID)virtAddMap.Flags;
}

PVOID identity::MapIdentityUntracked(CR3 cr3)
{
	if (!bIdentityCreated) {
		return (PVOID)MAXULONG64;
	}

	PPML4T ppml4 = (PPML4T)(cr3.AddressOfPageDirectory * PAGE_SIZE);

	bool bEntryFound = false;
	int pml4Idx = 0x80;
	for (; pml4Idx < 255; pml4Idx++) {
		if (ppml4->entry[pml4Idx].Flags == 0) {
			bEntryFound = true;
			break;
		}
	}
	if (!bEntryFound) {
		return (PVOID)MAXULONG64;
	}

	VIRT_ADD_MAP virtAddMap = { 0 };
	virtAddMap.Level4 = pml4Idx;
	return (PVOID)virtAddMap.Flags;
}

UINT64 identity::LastMappedPml4Index()
{
	return lastMappedIndex;
}

CR3 identity::LastMappedCR3()
{
	return lastMappedCR3;
}

void identity::ResetCache(bool bVmxRoot)
{
	if (!lastMappedCR3.Flags)
		return;

	PPML4T pml4 = (PPML4T)(lastMappedCR3.AddressOfPageDirectory * PAGE_SIZE);
	pml4->entry[lastMappedIndex].Flags = 0;

	lastMappedIndex = MAXULONG64;
	lastMappedCR3 = { 0 };
}

void identity::ResetCacheUntracked(CR3 cr3)
{
	if (!bIdentityCreated) {
		return;
	}

	PPML4T ppml4 = (PPML4T)(cr3.AddressOfPageDirectory * PAGE_SIZE);
	int pml4Idx = 0x80;
	for (; pml4Idx < 255; pml4Idx++) {
		if (ppml4->entry[pml4Idx].Ignored2 == 0
			&& ppml4->entry[pml4Idx].PageFrameNumber) {
			break;
		}
	}

	ppml4->entry[pml4Idx].Flags = 0;
}

identity::PhysicalAccess::PhysicalAccess()
{
	cr3.Flags = __readcr3();
	pIdentity = (char*)MapIdentityUntracked(cr3);
	bAllocated = true;
}

identity::PhysicalAccess::PhysicalAccess(UINT64 cr3)
{
	this->cr3.Flags = cr3;
	pIdentity = (char*)MapIdentityUntracked(this->cr3);
	bAllocated = true;
}

identity::PhysicalAccess::PhysicalAccess(PVOID identity, UINT64 _cr3)
{
	cr3.Flags = _cr3;
	pIdentity = (char*)identity;
	bAllocated = false;
}

identity::PhysicalAccess::~PhysicalAccess()
{
	if (bAllocated)
		ResetCacheUntracked(cr3);
}

void identity::_IDENTITY_MAPPING::Init()
{
	auto mapping = this;
	RtlZeroMemory(mapping, sizeof(*mapping));

	mapping->pml4[0].Present = true;
	mapping->pml4[0].Write = true;
	mapping->pml4[0].Supervisor = true;
	mapping->pml4[0].PageFrameNumber = (UINT64)&mapping->pdpt[0] / PAGE_SIZE;

	for (UINT64 EntryIndex = 0; EntryIndex < 512; EntryIndex++)
	{
		mapping->pdpt[EntryIndex].Present = true;
		mapping->pdpt[EntryIndex].Write = true;
		mapping->pdpt[EntryIndex].Supervisor = true;
		mapping->pdpt[EntryIndex].PageFrameNumber = (UINT64)&mapping->pdt[EntryIndex][0] / PAGE_SIZE;
	}

	for (UINT64 EntryGroupIndex = 0; EntryGroupIndex < 512; EntryGroupIndex++)
	{
		for (UINT64 EntryIndex = 0; EntryIndex < 512; EntryIndex++)
		{
			mapping->pdt[EntryGroupIndex][EntryIndex].Present = true;
			mapping->pdt[EntryGroupIndex][EntryIndex].Write = true;
			mapping->pdt[EntryGroupIndex][EntryIndex].LargePage = true;
			mapping->pdt[EntryGroupIndex][EntryIndex].Supervisor = true;
			mapping->pdt[EntryGroupIndex][EntryIndex].PageFrameNumber = (EntryGroupIndex * 512) + EntryIndex;
		}
	}

	mapping->pa = (UINT64)mapping->pml4;
}
