#pragma once

typedef unsigned char       UINT8;
typedef unsigned short      UINT16;
typedef unsigned int        UINT32;
typedef unsigned long long  UINT64;

typedef void* PVOID;

typedef unsigned char       BYTE;
typedef BYTE  BOOLEAN;
typedef BOOLEAN* PBOOLEAN;

#ifndef VOID
#define VOID void
typedef char CHAR;
typedef short SHORT;
typedef long LONG;
#if !defined(MIDL_PASS)
typedef int INT;
#endif
#endif

#define EXCEPTION_EXECUTE_HANDLER 1

#define min(a, b) (a > b ? b : a)

#define MAXULONG64 0xffffffffffffffff
