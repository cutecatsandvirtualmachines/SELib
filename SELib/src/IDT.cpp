#include <SELib/IDT.h>
#include <SELib/Bitmap.h>
#include <SELib/Memory.h>

IDTGateDescriptor64* pIDT = nullptr;

void IDTGateDescriptor64::setup(void* handler, bool present, UINT32 selector, UINT32 type)
{
    const auto handler_address = (UINT64)(handler);

    bits.offset0_15 = bitmap::bits<UINT32>(handler_address, 0, 15);
    bits.offset16_31 = bitmap::bits<UINT32>(handler_address, 16, 31);
    bits.offset32_63 = bitmap::bits<UINT32>(handler_address, 32, 63);
    bits.cs_selector = selector;
    bits.type = type;
    bits.present = present;
    bits.must_be_zero1 = 0;
    bits.ist = 0;
    bits.dpl = 0;
    bits.reserved = 0;
}

void IDTGateDescriptor64::setup(void* handler)
{
    const auto handler_address = (UINT64)(handler);

    bits.offset0_15 = bitmap::bits<UINT32>(handler_address, 0, 15);
    bits.offset16_31 = bitmap::bits<UINT32>(handler_address, 16, 31);
    bits.offset32_63 = bitmap::bits<UINT32>(handler_address, 32, 63);
}

UINT64 IDTGateDescriptor64::getAddress()
{
	return (UINT64)bits.offset32_63 << 32 | (UINT64)bits.offset16_31 << 16 | (UINT64)bits.offset0_15;
}

void IDT::setup(void(*handler)(), void(*handler_ecode)())
{
    setup_entry(0, true, handler);
    setup_entry(1, true, handler);
    setup_entry(3, true, handler);
    setup_entry(4, true, handler);
    setup_entry(5, true, handler);
    setup_entry(6, true, handler);
    setup_entry(7, true, handler);
    setup_entry(8, true, handler_ecode);
    setup_entry(9, true, handler);
    setup_entry(10, true, handler_ecode);
    setup_entry(11, true, handler_ecode);
    setup_entry(12, true, handler_ecode);
    setup_entry(13, true, handler_ecode);
    setup_entry(14, true, handler_ecode);
    setup_entry(15, false);
    setup_entry(16, true, handler);
    setup_entry(17, true, handler_ecode);
    setup_entry(18, true, handler);
    setup_entry(19, true, handler);
    setup_entry(20, true, handler);
    setup_entry(21, false);
    setup_entry(22, false);
    setup_entry(23, false);
    setup_entry(24, false);
    setup_entry(25, false);
    setup_entry(26, false);
    setup_entry(27, false);
    setup_entry(28, false);
    setup_entry(29, false);
    setup_entry(30, false);
    setup_entry(31, false);
}

void IDT::setup()
{
    RtlZeroMemory(descriptor, sizeof(descriptor));
    setup(generic_interrupt_handler, generic_interrupt_handler_ecode);
}

void IDT::setup(IDTGateDescriptor64* pOrigIDT)
{
    pIDT = pOrigIDT;
    RtlCopyMemory(descriptor, pOrigIDT, sizeof(descriptor));
}

void IDT::save()
{
    memory::memcpy(pIDT, descriptor, 20 * sizeof(IDTGateDescriptor64));
}
