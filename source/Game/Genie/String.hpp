#pragma once
#include <atomic>

inline auto Genie_String_Assign = (void(__thiscall*)(void*, const char*))(0x005ff4e0);
inline auto Genie_String_Constructor = (void(__thiscall*)(void*))(0x005ff660);
inline auto Genie_String_Constructor_WithStr = (void(__thiscall*)(void*, const char*))(0x005ff620);
inline auto Genie_String_Destructor = (void(__thiscall*)(void*))(0x005ff4a0);
inline auto Genie_String_Append = (void*(__thiscall*)(void*, const char*))(0x006130c0);
inline auto Genie_String_Delete = (int(__thiscall*)(void*, int, int))(0x0060e7d0);

namespace Genie {
		struct StringData {
			int len;
			int capacity;
			std::atomic<int> ref_count;
		};
		static_assert(sizeof(StringData) == 0xC);

	class String {
	public:
		char* data;
	public:
		inline String() {
			data = nullptr;
			Genie_String_Constructor(this);
		}
		inline String(const char* _str) {
			data = nullptr;
			Genie_String_Constructor_WithStr(this, _str);
		}
		inline ~String() {
			Genie_String_Destructor(this);
			data = nullptr;
		}
		inline StringData* GetData() {
			return reinterpret_cast<StringData*>(reinterpret_cast<std::uintptr_t>(data) - sizeof(StringData));
		}
		inline void Append(const char* lit) {
			Genie_String_Append(this, lit);
		}
		inline int Delete(int start, int count = 1) {
			return Genie_String_Delete(this, start, count);
		}
		inline int Length() {
			return GetData()->len;
		}
		String(const String&) = delete;
		String& operator=(const String&) = delete;
	};
};
