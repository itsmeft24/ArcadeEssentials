#pragma once
#include <utility>

inline auto AudioEventManager_SetVolume = (void(__thiscall*)(void*, int, float, float))(0x007e0c30);
inline auto AudioEventManager_GetVolume = (float(__thiscall*)(void*, int))(0x007e0cc0);
inline auto AudioEventManager_Pause = (void(__thiscall*)(void*, bool))(0x007e4640);
inline auto AudioEventManager_Unpause = (void(__thiscall*)(void*, bool))(0x007e4720);
inline auto** g_AudioEventManager = reinterpret_cast<class AudioEventManager**>(0x018cf310);

class AudioEventManager {
public:
	enum class Category : int {
		Master = 0, // Unconfirmed
		Sfx = 1, // Unconfirmed
		Sfx2D = 2,
		Sfx3D = 3,
		Dialog = 4, // Unconfirmed
		Music = 5,
	};
private:
	char unknown[0xA04];
public:
	inline void SetVolume(Category category, float volume, float fade) {
		AudioEventManager_SetVolume(this, std::to_underlying(category), volume, fade);
	}

	inline float GetVolume(Category category) {
		return AudioEventManager_GetVolume(this, std::to_underlying(category));
	}

	inline void Pause(bool enable) {
		AudioEventManager_Pause(this, enable);
	}
	
	inline void Unpause(bool enable) {
		AudioEventManager_Unpause(this, enable);
	}
};