#include <cstdint>
#include <utility>
#include "../Game/GameSpecificFlashImpl.hpp"

struct Resolution {
	unsigned int width;
	unsigned int height;
	static inline Resolution current() {
		bool windowed = *reinterpret_cast<bool*>(*reinterpret_cast<std::uintptr_t*>(0x019062c8) + 0x4E);
		D3DPRESENT_PARAMETERS* swap_chain_desc = windowed ? reinterpret_cast<D3DPRESENT_PARAMETERS*>(*reinterpret_cast<std::uintptr_t*>(0x019062c8) + 0x80) : reinterpret_cast<D3DPRESENT_PARAMETERS*>(*reinterpret_cast<std::uintptr_t*>(0x019062c8) + 0xB8);
		return Resolution{ swap_chain_desc->BackBufferWidth, swap_chain_desc->BackBufferHeight };
	}
	static inline Resolution configured() {
		return Resolution{ static_cast<std::uint32_t>(GLOBAL_CONFIG->window_width), static_cast<std::uint32_t>(GLOBAL_CONFIG->window_height) };
	}
	bool operator==(const Resolution& other) {
		if (width == other.width && height == other.height) {
			return true;
		}
		return false;
	}
};

void HandleGetAllResolutions(void* movie);
void HandleGetCurrResolution(void* movie);
void HandleSetCurrResolution(void* movie, float index);
void HandleGetCurrGraphicType(void* movie);
void HandleSetCurrGraphicType(void* movie, float index);
void HandleGetCurrLayoutType(void* movie);
void HandleSetCurrLayoutType(void* movie, float vsync);
void HandleGetMotionBlurEnabled(void* movie);
void HandleSetMotionBlurEnabled(void* movie, float motion_blur);
void HandleGetMenuOptionsList(void* movie);
void HandleSelectOption(void* movie);
void HandlePopupMessage(void* movie);