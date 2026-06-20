#pragma once
#include "../../Input/ControlMapper.hpp"

inline auto FlashControlMapper_SetActionMapPlatform = (void(__thiscall*)(void*, const char*))(0x011640c0);

namespace Flash {
	class FlashControlMapper : public ControlMapper {
	public:
		float timer;
		uint32_t buttonDownDelay;
		uint32_t buttonWaitTime;
		bool applyDelay;
		Vector2 curPos;
		float lastStickX;
		float lastStickY;
		void* actionMap;
	public:
		inline void SetActionMapPlatform(const char* platform) {
			FlashControlMapper_SetActionMapPlatform(this, platform);
		}
	};
};

static_assert(sizeof(Flash::FlashControlMapper) == 0x16c);