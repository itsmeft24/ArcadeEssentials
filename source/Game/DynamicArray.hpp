#pragma once

template <typename T> class DynamicArray {
public:
	T* m_Items;
	unsigned int m_Count;
	unsigned int m_Total;
	unsigned int m_Grow;
	bool m_Zero;
	bool m_Locked;
	unsigned int m_Align;
	void* m_Heap;
public:
	inline T& operator[](unsigned int index) {
		return m_Items[index];
	}
	inline const T& operator[](unsigned int index) const {
		return m_Items[index];
	}
	inline unsigned int Count() const {
		return m_Count;
	}
};

static_assert(sizeof(DynamicArray<void*>) == 0x1C);