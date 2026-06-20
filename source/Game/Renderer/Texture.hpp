#pragma once
#include <utility>
#include "../../Game/Types.hpp"

inline auto Renderer_Texture_Constructor = (void* (__thiscall*)(void*))(0x00868560);
inline auto Renderer_Texture_Create = (void* (_cdecl*)(void*))(0x0086ba60);
inline auto Renderer_Texture_Lock = (void*(__thiscall*)(void*, int, unsigned int*))(0x0086bba0);
inline auto Renderer_Texture_Unlock = (void(__thiscall*)(void*))(0x0086bd00);
inline auto Renderer_Texture_Destructor = (void* (__thiscall*)(void*))(0x00868690);
inline auto Renderer_Texture_SetFormat = (void (__thiscall*)(void*, unsigned int))(0x00869640);
inline auto Renderer_Texture_SetHardwareSwizzle = (void (__thiscall*)(void*, bool))(0x00869290);
inline auto Renderer_Texture_SetWidth = (void (__thiscall*)(void*, unsigned int))(0x00869490);
inline auto Renderer_Texture_SetHeight = (void (__thiscall*)(void*, unsigned int))(0x00869520);

namespace Renderer {
	enum class LockType {
		Invalid = 0,
		ReadOnly = 1,
		ReadWrite = 2,
		OverwriteAll = 3,
		OverwriteSome = 4
	};

	class Texture {
	public:
		enum class Format : unsigned int {
			Invalid = 0,
			A8R8G8B8 = 7,
			L8 = 18,
		};
		enum class MultisampleMode : unsigned int {
			None = 0,
		};
	public:
		char reserved[0x38] = {};
	public:
		inline Texture() {
			Renderer_Texture_Constructor(this);
		}

		static inline Types::SmartPointer<Texture> Create() {
			Texture* texture = nullptr;
			Renderer_Texture_Create(&texture);
			return Types::SmartPointer<Texture>::FromRaw(texture);
		}

		Texture(const Texture&) = delete;
		
		Texture& operator=(const Texture&) = delete;

		inline ~Texture() {
			Renderer_Texture_Destructor(this);
		}

		inline void* Lock(LockType lockType, unsigned int* outPitch) {
			return Renderer_Texture_Lock(this, std::to_underlying(lockType), outPitch);
		}

		inline void Unlock() {
			Renderer_Texture_Unlock(this);
		}

		inline void SetFormat(Format format) {
			Renderer_Texture_SetFormat(this, std::to_underlying(format));
		}
		
		inline void SetHardwareSwizzle(bool swizzle) {
			Renderer_Texture_SetHardwareSwizzle(this, swizzle);
		}
		
		inline void SetWidth(unsigned int width) {
			Renderer_Texture_SetWidth(this, width);
		}

		inline void SetHeight(unsigned int height) {
			Renderer_Texture_SetHeight(this, height);
		}
	};
};