#pragma once

#include <SELib/ia32.h>
#include <SELib/CPU.h>
#include <SELib/Bitmap.h>
#include "Memory.h"

typedef struct _IDT_REGS_ECODE IDT_REGS_ECODE, * PIDT_REGS_ECODE;
typedef struct _IDT_REGS IDT_REGS, * PIDT_REGS;

extern "C" void generic_interrupt_handler_vm();
extern "C" void generic_interrupt_handler_ecode_vm();
extern "C" void __nmi_handler_vm();
extern "C" void __gp_handler_vm();
extern "C" void __pf_handler_vm();
extern "C" void __de_handler_vm();

extern "C" void seh_handler_ecode_vm(PIDT_REGS_ECODE regs);
extern "C" void seh_handler_vm(PIDT_REGS regs);

// Vol3 Ch 6.14.1
union IDTGateDescriptor64
{
private:
public:
    struct
    {
        UINT32 offset0_15 : 16;
        UINT32 cs_selector : 16;
        UINT32 ist : 3;
        UINT32 must_be_zero1 : 5;
        UINT32 type : 5;
        UINT32 dpl : 2;
        UINT32 present : 1;
        UINT32 offset16_31 : 16;
        UINT32 offset32_63 : 32;
        UINT32 reserved : 32;
    } bits;
    UINT32 values[4]{};

    constexpr IDTGateDescriptor64() = default;

    void setup(void* handler, bool present, UINT32 selector, UINT32 type)
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

    void setup(void* handler)
    {
        const auto handler_address = (UINT64)(handler);

        bits.offset0_15 = bitmap::bits<UINT32>(handler_address, 0, 15);
        bits.offset16_31 = bitmap::bits<UINT32>(handler_address, 16, 31);
        bits.offset32_63 = bitmap::bits<UINT32>(handler_address, 32, 63);
    }

    UINT64 getAddress()
    {
        return (UINT64)bits.offset32_63 << 32 | (UINT64)bits.offset16_31 << 16 | (UINT64)bits.offset0_15;
    }
};

typedef struct _IDT_REGS_ECODE
{
    UINT64 r15;
    UINT64 r14;
    UINT64 r13;
    UINT64 r12;
    UINT64 r11;
    UINT64 r10;
    UINT64 r9;
    UINT64 r8;
    UINT64 rbp;
    UINT64 rdi;
    UINT64 rsi;
    UINT64 rdx;
    UINT64 rcx;
    UINT64 rbx;
    UINT64 rax;

    UINT64 error_code;
    UINT64 rip;
    UINT64 cs_selector;
    RFLAGS rflags;
    UINT64 rsp;
    UINT64 ss_selector;
} IDT_REGS_ECODE, * PIDT_REGS_ECODE;

typedef struct _IDT_REGS
{
    UINT64 r15;
    UINT64 r14;
    UINT64 r13;
    UINT64 r12;
    UINT64 r11;
    UINT64 r10;
    UINT64 r9;
    UINT64 r8;
    UINT64 rbp;
    UINT64 rdi;
    UINT64 rsi;
    UINT64 rdx;
    UINT64 rcx;
    UINT64 rbx;
    UINT64 rax;

    UINT64 rip;
    UINT64 cs_selector;
    RFLAGS rflags;
    UINT64 rsp;
    UINT64 ss_selector;
} IDT_REGS, * PIDT_REGS;

class IDT
{
public:
    IDTGateDescriptor64 descriptor[256];

    void setup_entry(size_t i, bool present, void* handler = nullptr)
    {
        descriptor[i].setup(
            handler,
            present,
            CPU::GetCs(),
            SEGMENT_DESCRIPTOR_TYPE_INTERRUPT_GATE
        );
    }

    void setup_entry(size_t i, void* handler = nullptr)
    {
        descriptor[i].setup(
            handler
        );
    }

    void* get_address() { return &descriptor; }
    size_t get_limit() const { return sizeof(descriptor) | 7; }

    void setup(void(*handler)(), void(*handler_ecode)())
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

    void setup()
    {
        RtlZeroMemory(descriptor, sizeof(descriptor));
        setup(generic_interrupt_handler_vm, generic_interrupt_handler_ecode_vm);
    }
};
