#pragma once

#include <Windows.h>

namespace vmcall {
	bool IsVmcall(UINT64 r9);
	bool IsValidKey(UINT64 key);

	void SetKey(UINT64 key);
	UINT64 GetKey();
}