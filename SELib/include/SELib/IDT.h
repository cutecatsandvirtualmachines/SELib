#pragma once

#include <SELib/ia32.h>
#include <SELib/CPU.h>

typedef struct _IDT_REGS_ECODE IDT_REGS_ECODE, * PIDT_REGS_ECODE;
typedef struct _IDT_REGS IDT_REGS, * PIDT_REGS;

extern "C" void generic_interrupt_handler();
extern "C" void generic_interrupt_handler_ecode();
extern "C" void __gp_handler();
extern "C" void __pf_handler();
extern "C" void __de_handler();
extern "C" void __db_handler();

extern "C" void seh_handler_ecode(PIDT_REGS_ECODE regs);
extern "C" void seh_handler(PIDT_REGS regs);

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

    void setup(void* handler, bool present, UINT32 selector, UINT32 type);
    void setup(void* handler);
    UINT64 getAddress();
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

    void setup(void(*handler)(), void(*handler_ecode)());
    void setup();
    void setup(IDTGateDescriptor64* pOrigIDT);

    void save();
};
