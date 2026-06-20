#pragma once
#include <cstdint>
#include <Windows.h>
#include "Kernel.hpp"
#include "DynamicArray.hpp"

using u64 = std::uint64_t;
using s64 = std::int64_t;

using u32 = std::uint32_t;
using s32 = std::int32_t;

using u16 = std::uint16_t;
using s16 = std::int16_t;

using u8 = std::uint8_t;
using s8 = std::int8_t;

inline auto	Types_GarbageCollector_Register = (void(_cdecl*)(void*))(0x007753d0);
inline auto	Types_SmartPointerArray_Clear = (void(__thiscall*)(void*, bool))(0x00c9c6b0);

namespace Types {

	class ReferenceCounter;
	class GarbageCollectible;

	template <class T> class SmartPointer {
	private:
		T* target;
	private:
		inline SmartPointer(T* raw) { target = raw; }
	public:
		inline SmartPointer() { target = nullptr; }

		SmartPointer(const SmartPointer&) = delete;
		SmartPointer& operator=(const SmartPointer&) = delete;

		inline ~SmartPointer() {
			if (target != nullptr) {
				ReferenceCounter* counter = reinterpret_cast<ReferenceCounter*>(target);
				if (counter != nullptr) {
					counter->DecrementReferenceCount();
				}
			}
		}

		constexpr inline operator bool() {
			return target != nullptr;
		}

		constexpr inline T* operator->() const {
			return target;
		}

		inline T& operator*() const {
			return *target;
		}

		constexpr inline T* Get() const {
			return target;
		}

		static inline SmartPointer<T> FromRaw(T* raw) {
			return { raw };
		}

		inline T* Leak() {
			T* value = target;
			target = nullptr;
			return value;
		}
	};

	class GarbageCollector {
	public:
		static void Register(GarbageCollectible* _ptr) {
			Types_GarbageCollector_Register(_ptr);
		}
	};

	class ReferenceCounter {
	public:
		enum Flags {
			GarbageCollectible = 0x80000000,
			GarbageCollected = 0x40000000,
			FlagsMask = 0xC0000000,
			CategoryMask = 0x3FC00000,
			CategoryShift = 22,
			RefCountMask = ~(FlagsMask | CategoryMask)
		};

		mutable volatile u32 ref_count;
	public:
		inline ReferenceCounter() : ref_count(0) {
			// Set VTable.
			*reinterpret_cast<std::uintptr_t*>(this) = 0x015f253c;
		}
		inline virtual ~ReferenceCounter() {}
		inline void DecrementReferenceCount() {
			u32 _ref_count = _InterlockedDecrement(&ref_count);
			if ((_ref_count & Flags::RefCountMask) == 0) {
				if ((_ref_count & Flags::GarbageCollectible) != 0) {
					GarbageCollector::Register(reinterpret_cast<Types::GarbageCollectible*>(this));
				}
				else {
					auto deleter = **reinterpret_cast<void*(__thiscall***)(void*, unsigned int)>(this);
					deleter(this, 1);
				}
			}
		}
	};

	class SmartHandleIndirection : public ReferenceCounter {
	public:
		mutable volatile void* pointer;
		Kernel::Thread::SpinLock pointer_lock;
	public:
	};

	class ReferenceCountable : public ReferenceCounter {
	public:
		SmartPointer<SmartHandleIndirection> indirection;
	public:
		inline virtual ~ReferenceCountable() override {
		}
	};

	class GarbageCollectible : public ReferenceCountable {
	public:
		inline virtual ~GarbageCollectible() override {
		}
		inline virtual void Dispose() {
		}
	};

	template <class T> class SmartPointerArray : public DynamicArray<SmartPointer<T>> {
	public:
		inline void Clear(bool freeArrayMemory = false) {
			Types_SmartPointerArray_Clear(this, freeArrayMemory);
		}
	};

	static_assert(sizeof(ReferenceCounter) == 8);
	static_assert(sizeof(ReferenceCountable) == 12);
	static_assert(sizeof(GarbageCollectible) == 12);
};