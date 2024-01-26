#pragma once

#define DbgMsg(x, ...) Print((CHAR16*)(x L"\n"), __VA_ARGS__)

namespace io {
	namespace vga {
		void Clear();
		void Output(wchar_t* str);
	}
}