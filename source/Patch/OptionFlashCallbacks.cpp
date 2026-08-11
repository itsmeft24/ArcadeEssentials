#include <iostream>
#include <utility>
#include <vector>
#include <cstdint>
#include <string>
#include <d3d9.h>
#include "../Game/MiscOffsets.hpp"
#include "../Game/GameSpecificFlashImpl.hpp"
#include "../Game/Scaleform/GFxValue.hpp"
#include "../Game/Genie/String.hpp"
#include "../Game/FrontEnd/CarsFrontEnd.hpp"
#include "../config.hpp"
#include "OptionFlashCallbacks.hpp"

inline bool windowed_mode() {
	return *reinterpret_cast<bool*>(*reinterpret_cast<std::uintptr_t*>(0x019062c8) + 0x4E);
}

static std::vector<Resolution> VALID_TARGETS{};
static bool VALID_TARGETS_FOUND = false;

void HandleGetAllResolutions(void* movie) {
	// Lazy-initialize VALID_TARGETS.
	if (!VALID_TARGETS_FOUND) {
		IDirect3D9* d3d = Direct3DCreate9(D3D_SDK_VERSION);
		if (d3d != nullptr) {
			for (std::uint32_t i = 0; i < d3d->GetAdapterModeCount(0, D3DFMT_X8R8G8B8); ++i) {
				D3DDISPLAYMODE mode;
				if (SUCCEEDED(d3d->EnumAdapterModes(0, D3DFMT_X8R8G8B8, i, &mode))) {
					Resolution found = { mode.Width, mode.Height };
					if (std::find(VALID_TARGETS.begin(), VALID_TARGETS.end(), found) == VALID_TARGETS.end()) {
						VALID_TARGETS.push_back(found);
					}
				}
			}
			d3d->Release();
		}
		else {
			VALID_TARGETS.push_back(Resolution::configured());
		}
		VALID_TARGETS_FOUND = true;
	}

	// Formulate and return a list of valid resolutions the game will accept.
	std::string return_string = "";
	for (std::size_t i = 0; i < VALID_TARGETS.size(); i++) {
		return_string += std::to_string(VALID_TARGETS[i].width) + "x" + std::to_string(VALID_TARGETS[i].height);
		if (i != VALID_TARGETS.size() - 1) {
			return_string += ",";
		}
	}
	// Pass the list to the game through a Value.
	GFxValue data(return_string.data());
	auto* inst = reinterpret_cast<std::uintptr_t*>(movie);
	auto func = *reinterpret_cast<std::uint32_t(__thiscall**)(void*, GFxValue*)>(*inst + 200);
	// Set the return value for the Flash function.
	func(inst, &data);
	return;
}

void HandleGetCurrResolution(void* movie) {
	// It *should* be impossible for `configured` to not be inside the VALID_TARGETS list.
	Resolution configured{ static_cast<std::uint32_t>(GLOBAL_CONFIG->window_width), static_cast<std::uint32_t>(GLOBAL_CONFIG->window_height) };
	int index = std::find(VALID_TARGETS.begin(), VALID_TARGETS.end(), configured) - VALID_TARGETS.begin();

	// Pass the resolution index to the game through a Value.
	GFxValue data(static_cast<float>(index));
	auto* inst = reinterpret_cast<std::uintptr_t*>(movie);
	auto func = *reinterpret_cast<std::uint32_t(__thiscall**)(void*, GFxValue*)>(*inst + 200);
	// Set the return value for the Flash function.
	func(inst, &data);
	return;
}

void HandleSetCurrResolution(void* movie, float index) {
	const auto& new_target = VALID_TARGETS[static_cast<int>(index)];
	GLOBAL_CONFIG->window_width = new_target.width;
	GLOBAL_CONFIG->window_height = new_target.height;
	GLOBAL_CONFIG->finalize();
}

void HandleGetCurrGraphicType(void* movie) {
	// Pass the 'graphic type index' (really our shadow quality) to the game through a Value.
	GFxValue data(static_cast<float>(std::to_underlying(GLOBAL_CONFIG->shadow_quality)));
	auto* inst = reinterpret_cast<std::uintptr_t*>(movie);
	auto func = *reinterpret_cast<std::uint32_t(__thiscall**)(void*, GFxValue*)>(*inst + 200);
	// Set the return value for the Flash function.
	func(inst, &data);
}

void HandleSetCurrGraphicType(void* movie, float index) {
	GLOBAL_CONFIG->shadow_quality = static_cast<ShadowQuality>(index);
}

void HandleGetCurrLayoutType(void* movie) {
	// Pass the 'layout type index' (really if vsync is enabled) to the game through a Value.
	GFxValue data(static_cast<float>(GLOBAL_CONFIG->vsync ? 1 : 0));
	auto* inst = reinterpret_cast<std::uintptr_t*>(movie);
	auto func = *reinterpret_cast<std::uint32_t(__thiscall**)(void*, GFxValue*)>(*inst + 200);
	// Set the return value for the Flash function.
	func(inst, &data);
}

void HandleSetCurrLayoutType(void* movie, float vsync) {
	GLOBAL_CONFIG->vsync = vsync != 0.0f;
	GLOBAL_CONFIG->finalize();
}

void HandleGetMotionBlurEnabled(void* movie) {
	// Pass the motion blur setting index (which is just if it's enabled or not) to the game through a Value.
	GFxValue data(static_cast<float>(GLOBAL_CONFIG->motion_blur ? 1 : 0));
	auto* inst = reinterpret_cast<std::uintptr_t*>(movie);
	auto func = *reinterpret_cast<std::uint32_t(__thiscall**)(void*, GFxValue*)>(*inst + 200);
	// Set the return value for the Flash function.
	func(inst, &data);
}

void HandleSetMotionBlurEnabled(void* movie, float motion_blur) {
	GLOBAL_CONFIG->motion_blur = motion_blur != 0.0f;
	GLOBAL_CONFIG->finalize();
}

void HandleGetMenuOptionsList(void* movie) {
	// Pass the string to the game through a Value.
	GFxValue data("SharedText_OK");
	auto* inst = reinterpret_cast<std::uintptr_t*>(movie);
	auto func = *reinterpret_cast<std::uint32_t(__thiscall**)(void*, GFxValue*)>(*inst + 200);
	// Set the return value for the Flash function.
	func(inst, &data);
}

void HandleSelectOption(void* movie) {
	ErrorPopup_MarkForClose(*g_ErrorPopup);
	if ((*g_FrontEnd)->primaryMovie != nullptr) {
		(*g_FrontEnd)->primaryMovie->CallFlashFunction("OnMessageOk", nullptr);
	}
}

void HandlePopupMessage(void* movie) {
	std::uintptr_t* inst = *g_ErrorPopup;
	auto func = *reinterpret_cast<bool(__thiscall**)(std::uintptr_t*, const char*)>(*inst + 0x20);
	auto func_2 = *reinterpret_cast<bool(__thiscall**)(std::uintptr_t*, const char*, int)>(*inst + 0x24);
	auto func_3 = *reinterpret_cast<bool(__thiscall**)(std::uintptr_t*, const char*)>(*inst + 0x1C);
	auto func_4 = *reinterpret_cast<bool(__thiscall**)(std::uintptr_t*, int)>(*inst + 0x14);
	func(inst, "Win32Wii_Msg_Title");
	if (windowed_mode()) {
		func_2(inst, "Win32Wii_Msg_ApplySettings_Windowed", 1);
	}
	else {
		func_2(inst, "Win32Wii_Msg_ApplySettings", 1);
	}
	func_3(inst, "SharedText_OK");
	func_4(inst, 3);
}