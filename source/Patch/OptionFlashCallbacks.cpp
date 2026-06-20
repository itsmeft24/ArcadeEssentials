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
	std::uintptr_t front_end = *reinterpret_cast<std::uintptr_t*>(0x0192b8ac);
	std::uintptr_t _movie = *reinterpret_cast<std::uintptr_t*>(front_end + 0xb8);
	if (_movie != 0) {
		Flash_Movie_CallFlashFunction(_movie, "OnMessageOk", nullptr);
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

/*
OptionFlashCallbacks::OptionFlashCallbacks() {
}

OptionFlashCallbacks::~OptionFlashCallbacks() {
}

void OptionFlashCallbacks::AttachLayer() {
	get_options_list.callbacks_list = this;
	select_option.callbacks_list = this;
	go_back.callbacks_list = this;
	get_option_val.callbacks_list = this;
	set_option_val.callbacks_list = this;
	get_options_list_types.callbacks_list = this;
	get_all_resolutions.callbacks_list = this;
	get_curr_resolution.callbacks_list = this;
	set_curr_resolution.callbacks_list = this;
	get_curr_graphic_type.callbacks_list = this;
	set_curr_graphic_type.callbacks_list = this;
	get_curr_layout_type.callbacks_list = this;
	set_curr_layout_type.callbacks_list = this;
	get_curr_brightness.callbacks_list = this;
	set_curr_brightness.callbacks_list = this;
	get_key_from_mapper.callbacks_list = this;
	start_set_key_in_mapper.callbacks_list = this;
	set_key_in_mapper.callbacks_list = this;
	reset_keys_in_mapper.callbacks_list = this;
	get_title.callbacks_list = this;
	get_item_name.callbacks_list = this;
	get_all_items.callbacks_list = this;
	get_curr_item.callbacks_list = this;
	set_curr_item.callbacks_list = this;
	on_page_back.callbacks_list = this;
	update_trigger_by_state.callbacks_list = this;
	set_menu_id.callbacks_list = this;
	popup_message.callbacks_list = this;
	is_in_pause.callbacks_list = this;
	flash_functions.SetupFlashFunctions();
}

void OptionFlashCallbacks::Init(void* movie) {
	if (movie != nullptr) {
		this->unk = nullptr;
		this->parent_movie = movie;
		AttachLayer();
	}
}

OptionFlashCallbacks::FrontendFlashFunctions::FrontendFlashFunctions() {
	callbacks_list = nullptr;
}

OptionFlashCallbacks::FrontendFlashFunctions::~FrontendFlashFunctions() {
}

void OptionFlashCallbacks::FrontendFlashFunctions::SetupFlashFunctions() {
	SetFlashVariableFunc("GetMenuOptionsList", SetFlashVariDef::ArgumentType::Null, &callbacks_list->get_options_list);
	SetFlashVariableFunc("SelectOption", SetFlashVariDef::ArgumentType::String, &callbacks_list->select_option);
	SetFlashVariableFunc("GetOptionsListTypes", SetFlashVariDef::ArgumentType::Null, &callbacks_list->get_options_list_types);
	SetFlashVariableFunc("GetOptionVal", SetFlashVariDef::ArgumentType::String, &callbacks_list->get_option_val);
	SetFlashVariableFunc("SetOptionVal", SetFlashVariDef::ArgumentType::String, &callbacks_list->set_option_val);
	SetFlashVariableFunc("GoBack", SetFlashVariDef::ArgumentType::Null, &callbacks_list->go_back);
	SetFlashVariableFunc("GetAllResolutions", SetFlashVariDef::ArgumentType::Null, &callbacks_list->get_all_resolutions);
	SetFlashVariableFunc("GetCurrResolution", SetFlashVariDef::ArgumentType::Null, &callbacks_list->get_curr_resolution);
	SetFlashVariableFunc("SetCurrResolution", SetFlashVariDef::ArgumentType::Number, &callbacks_list->set_curr_resolution);
	SetFlashVariableFunc("GetCurrGraphicType", SetFlashVariDef::ArgumentType::Null, &callbacks_list->get_curr_graphic_type);
	SetFlashVariableFunc("SetCurrGraphicType", SetFlashVariDef::ArgumentType::Number, &callbacks_list->set_curr_graphic_type);
	SetFlashVariableFunc("GetCurrLayoutType", SetFlashVariDef::ArgumentType::Null, &callbacks_list->get_curr_layout_type);
	SetFlashVariableFunc("SetCurrLayoutType", SetFlashVariDef::ArgumentType::Number, &callbacks_list->set_curr_layout_type);
	SetFlashVariableFunc("GetCurrBrightness", SetFlashVariDef::ArgumentType::Null, &callbacks_list->get_curr_brightness);
	SetFlashVariableFunc("SetCurrBrightness", SetFlashVariDef::ArgumentType::Number, &callbacks_list->set_curr_brightness);
	SetFlashVariableFunc("GetKeyFromMapper", SetFlashVariDef::ArgumentType::Number, &callbacks_list->get_key_from_mapper);
	SetFlashVariableFunc("StartSetKeyInMapper", SetFlashVariDef::ArgumentType::Null, &callbacks_list->start_set_key_in_mapper);
	SetFlashVariableFunc("SetKeyInMapper", SetFlashVariDef::ArgumentType::Number, &callbacks_list->set_key_in_mapper);
	SetFlashVariableFunc("ResetKeysInMapper", SetFlashVariDef::ArgumentType::Null, &callbacks_list->reset_keys_in_mapper);
	SetFlashVariableFunc("GetTitle", SetFlashVariDef::ArgumentType::Null, &callbacks_list->get_title);
	SetFlashVariableFunc("GetItemName", SetFlashVariDef::ArgumentType::Null, &callbacks_list->get_item_name);
	SetFlashVariableFunc("GetAllItems", SetFlashVariDef::ArgumentType::Null, &callbacks_list->get_all_items);
	SetFlashVariableFunc("GetCurrItem", SetFlashVariDef::ArgumentType::Null, &callbacks_list->get_curr_item);
	SetFlashVariableFunc("SetCurrItem", SetFlashVariDef::ArgumentType::Number, &callbacks_list->set_curr_item);
	SetFlashVariableFunc("OnPageBack", SetFlashVariDef::ArgumentType::Null, &callbacks_list->on_page_back);
	SetFlashVariableFunc("UpdateTriggerByState", SetFlashVariDef::ArgumentType::Number, &callbacks_list->update_trigger_by_state);
	SetFlashVariableFunc("SetMenuId", SetFlashVariDef::ArgumentType::Number, &callbacks_list->set_menu_id);
	SetFlashVariableFunc("PopupMessage", SetFlashVariDef::ArgumentType::Number, &callbacks_list->popup_message);
	SetFlashVariableFunc("IsInPause", SetFlashVariDef::ArgumentType::Null, &callbacks_list->is_in_pause);
}

void OptionFlashCallbacks::GetOptionsList::operator()(void* val, void* movie) {}
void OptionFlashCallbacks::SelectOption::operator()(void* val, void* movie) {}
void OptionFlashCallbacks::GoBack::operator()(void* val, void* movie) {}
void OptionFlashCallbacks::GetOptionVal::operator()(void* val, void* movie) {}
void OptionFlashCallbacks::SetOptionVal::operator()(void* val, void* movie) {}
void OptionFlashCallbacks::GetOptionsListTypes::operator()(void* val, void* movie) {}
void OptionFlashCallbacks::GetAllResolutions::operator()(void* val, void* movie) {}
void OptionFlashCallbacks::GetCurrResolution::operator()(void* val, void* movie) {}
void OptionFlashCallbacks::SetCurrResolution::operator()(void* val, void* movie) {}
void OptionFlashCallbacks::GetCurrGraphicType::operator()(void* val, void* movie) {}
void OptionFlashCallbacks::SetCurrGraphicType::operator()(void* val, void* movie) {}
void OptionFlashCallbacks::GetCurrLayoutType::operator()(void* val, void* movie) {}
void OptionFlashCallbacks::SetCurrLayoutType::operator()(void* val, void* movie) {}
void OptionFlashCallbacks::GetCurrBrightness::operator()(void* val, void* movie) {}
void OptionFlashCallbacks::SetCurrBrightness::operator()(void* val, void* movie) {}
void OptionFlashCallbacks::GetKeyFromMapper::operator()(void* val, void* movie) {}
void OptionFlashCallbacks::StartSetKeyInMapper::operator()(void* val, void* movie) {}
void OptionFlashCallbacks::SetKeyInMapper::operator()(void* val, void* movie) {}
void OptionFlashCallbacks::ResetKeysInMapper::operator()(void* val, void* movie) {}
void OptionFlashCallbacks::GetTitle::operator()(void* val, void* movie) {}
void OptionFlashCallbacks::GetItemName::operator()(void* val, void* movie) {}
void OptionFlashCallbacks::GetAllItems::operator()(void* val, void* movie) {}
void OptionFlashCallbacks::GetCurrItem::operator()(void* val, void* movie) {}
void OptionFlashCallbacks::SetCurrItem::operator()(void* val, void* movie) {}
void OptionFlashCallbacks::OnPageBack::operator()(void* val, void* movie) {}
void OptionFlashCallbacks::UpdateTriggerByState::operator()(void* val, void* movie) {}
void OptionFlashCallbacks::SetMenuId::operator()(void* val, void* movie) {}
void OptionFlashCallbacks::PopupMessage::operator()(void* val, void* movie) {}
void OptionFlashCallbacks::IsInPause::operator()(void* val, void* movie) {}
*/