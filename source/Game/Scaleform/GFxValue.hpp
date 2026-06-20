#pragma once

inline auto GFxValueDestructor = (void* (__thiscall*)(void*))(0x005fb740);

struct GFxValue {
	struct ObjectInterface {};
	enum class ValueType : unsigned int {
		Undefined = 0,
		Null,
		Boolean,
		Number,
		String,
		StringW,
		Object,
		Array,
		DisplayObject
	};

	ObjectInterface* objectInterface;
	ValueType type;
	union {
		const char* string;
		float number;
	};
	inline GFxValue() : objectInterface(0), type(ValueType::Undefined), string(nullptr) {}
	inline GFxValue(const char* str) : objectInterface(0), type(ValueType::String), string(str) {}
	inline GFxValue(float num) : objectInterface(0), type(ValueType::Number), number(num) {}
	inline ~GFxValue() {
		GFxValueDestructor(this);
	}
	GFxValue(const GFxValue&) = delete;
	GFxValue& operator=(const GFxValue&) = delete;
};