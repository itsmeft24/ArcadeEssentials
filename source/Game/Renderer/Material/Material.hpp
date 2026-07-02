#pragma once
#include "../../Game/Types.hpp"

inline auto Renderer_Material_Constructor = (void* (__thiscall*)(void*))(0x00840a40);
inline auto Renderer_Material_Create = (void* (_cdecl*)(void*, const char*, unsigned int))(0x00840cb0);
inline auto Renderer_Material_Clone = (void* (__thiscall*)(void*, void*))(0x00844da0);
inline auto Renderer_Material_Destructor = (void* (__thiscall*)(void*))(0x00840bd0);
inline auto Renderer_Material_SetTexture = (void (__thiscall*)(void*, void*, const char*, int))(0x00847080);
inline auto Renderer_Material_GetTechnique = (void* (__thiscall*)(void*, const char*))(0x00841330);
inline auto Renderer_Material_SetTechnique = (void (__thiscall*)(void*, void*))(0x00841390);

namespace Renderer {
	class R_ShaderTechnique;
	class Material {
	private:
	public:
		inline Material() {
			Renderer_Material_Constructor(this);
		}

		static inline Types::SmartPointer<Material> Create(const char* effectName, unsigned int caps = 0) {
			Material* material = nullptr;
			Renderer_Material_Create(&material, effectName, caps);
			return Types::SmartPointer<Material>::FromRaw(material);
		}

		inline Types::SmartPointer<Material> Clone() {
			Material* material = nullptr;
			Renderer_Material_Clone(this, &material);
			return Types::SmartPointer<Material>::FromRaw(material);
		}

		Material(const Material&) = delete;

		Material& operator=(const Material&) = delete;

		inline ~Material() {
			Renderer_Material_Destructor(this);
		}

		inline void SetTexture(Texture* texture, const char* label, int unk) {
			Renderer_Material_SetTexture(this, texture, label, unk);
		}

		inline R_ShaderTechnique* GetTechnique(const char* name) {
			return reinterpret_cast<R_ShaderTechnique*>(Renderer_Material_GetTechnique(this, name));
		}

		inline void SetTechnique(R_ShaderTechnique* technique) {
			Renderer_Material_SetTechnique(this, technique);
		}
	};
};