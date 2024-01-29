#pragma once

namespace vmcall {
	__declspec(dllexport) inline unsigned long long vmcallKey;

	__forceinline bool IsVmcall(unsigned long long r9)
	{
		return r9 == (vmcallKey ^ 0xbabab00e);
	}

	__forceinline bool IsValidKey(unsigned long long _key)
	{
		return !vmcallKey || vmcallKey == _key;
	}

	__forceinline void SetKey(unsigned long long _key)
	{
		vmcallKey = _key;
	}

	__forceinline unsigned long long GetKey()
	{
		return vmcallKey;
	}
}