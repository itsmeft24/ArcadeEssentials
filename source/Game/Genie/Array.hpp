#pragma once
#include <cstdint>

namespace Genie {
	template <typename T> struct Array {
	private:
		void* allocator;
		T* data;
		std::uint32_t len;
		std::uint32_t capacity;
		std::uint32_t unknown;
	public:
		inline std::uint32_t Length() const {
			return len;
		}

		inline T& operator[](std::uint32_t i) {
			return data[i];
		}
		
		inline const T& operator[](std::uint32_t i) const {
			return data[i];
		}
	};
};