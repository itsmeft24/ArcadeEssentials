#include <iostream>
#include <fstream>
#include <filesystem>
#include <Windows.h>
#include <shlwapi.h>
#include <sunset.hpp>
#include <d3d9.h>
#include "pentane.hpp"
#include "Game/GameSpecificFlashImpl.hpp"
#include "Game/Genie/String.hpp"
#include "Game/GameProgressionManager.hpp"
#include "Game/Stage/StageEntity.hpp"
#include "Game/Components/ActiveMoves.hpp"
#include "Game/Components/CarsReactionMonitor.hpp"

static std::atomic<bool> IS_PC = false;
// static std::atomic<bool> IS_MACHINE_DESKTOP = false;

inline auto WindowsSystemInputDriver_LockPlayerToController = (void(__thiscall*)(std::uintptr_t, int, int))(0x00816dd0);
inline auto WindowsSystemInputDriver_IsControllerLocked = (bool(__thiscall*)(std::uintptr_t, int))(0x00816ed0);
inline auto WindowsSystemInputDriver_GetUnlockedController= (void*(__thiscall*)(std::uintptr_t, int))(0x00817050);
inline auto FUN_00ef3a30 = (void(__thiscall*)(std::uintptr_t))(0x00ef3a30);
inline auto FUN_0060e7d0 = (void(__thiscall*)(std::uintptr_t, int, int))(0x0060e7d0);
inline auto FUN_0060ee20 = (void**(__thiscall*)(std::uintptr_t, void*, int))(0x0060ee20);
inline auto FUN_00613000 = (void*(__thiscall*)(std::uintptr_t, void*))(0x00613000);
inline auto _CarsFrontEnd_SetScreen = (void(__thiscall*)(void*, int, const char*, bool))(0x004c1440);
inline auto _CarsFrontEnd_SetLevelAndUnk = (void(__thiscall*)(void*, void*))(0x004c0780);
inline auto _CarsFrontEnd_UnkHandleTrackLengthType = (void(__thiscall*)(void*, char*))(0x004c0800);
inline auto _CarsFrontEnd_SetGameModeIndex = (void(__thiscall*)(void*, char*))(0x004c2dc0);
inline auto _CarsFrontEnd_UNK_004c3b70 = (void(__thiscall*)(void*))(0x004c3b70);
inline auto Flash_Movie_CallFlashFunction = (void(__cdecl*)(std::uintptr_t, const char*, ...))(0x01168690);
inline auto Flash_EngineTextureLoader_LoadTextureSet = (void* (__thiscall*)(void*, const char*, bool, int))(0x0116cd20);
inline auto CTranslator_Translate = (char*(__thiscall*)(void*, void*, bool))(0x00cf6dd0);
inline auto UnkExcelDataBase_GetUnk = (void(__thiscall*)(void*, void*, void*))(0x0052cd70);
inline auto UnkExcelDataBase_GetUnk1 = (void(__thiscall*)(void*, void*, void*))(0x0052ce80);
inline auto UnkExcelDataBase_GetUnk2 = (void(__thiscall*)(void*, void*, void*, float*, float*))(0x0052cfe0);
inline auto UnkExcelDataBase_GetUnk3 = (void(__thiscall*)(void*, std::uint32_t, void*, void*))(0x0052af10);
inline auto PersistentData_GetGlobal = (int(__thiscall*)(void*, const char*))(0x00cf0e20);
inline auto PersistentData_SetGlobal = (void(__thiscall*)(void*, const char*, std::uint32_t))(0x00cf0e40);
inline auto FUN_00f675d0 = (std::uint32_t(__thiscall*)(void*, const char*, std::uint32_t))(0x00f675d0);
inline auto FUN_00f66d60 = (std::uint32_t(__thiscall*)(void*, std::uint32_t))(0x00f66d60);
inline auto GameCommon_SetPlayerSfxVolume = (void(__thiscall*)(void*, float, char))(0x00eb48b0);
inline auto GameCommon_SetPlayerMusicVolume = (void(__thiscall*)(void*, float, char))(0x00eb49a0);
inline auto GameCommon_SetPlayerDialogueVolume = (void(__thiscall*)(void*, float, char))(0x00eb4a90);
inline auto FUN_0116d4d0 = (void(__thiscall*)(std::uintptr_t, std::uintptr_t, const char*))(0x0116d4d0);
inline auto CSaveGame_UNK_00ee99a0 = (void(__thiscall*)(std::uintptr_t, bool))(0x00ee99a0);
inline auto CSaveGame_ClearLoadedData = (void(__thiscall*)(std::uintptr_t))(0x00ee9ae0);
inline auto FUN_00ba0870 = (void(__thiscall*)(std::uintptr_t, std::uintptr_t))(0x00ba0870);

enum CarsFrontEndScreen {
	Invalid = 0,
	ExitToWindows = 1,
	AutoSaveWarning = 2,
	TitleMenu = 3,
	UnkSubMenuGears = 4, // FIXME
	SaveFileLoading = 5,
	SaveSlots = 6, // Causes a game crash!
	MainMenu_Extras = 7,
	Extras_Options = 8,
	Extras_EnterCode = 9,
	Extras_Cheats = 10,
	Extras_Credits = 11,
	MT_FrontEnd = 12,
	MainMenu_StoryMissions = 13,
	MissionSelect = 14,
	MissionDetails = 15,
	MainMenu_Badges = 16,
	BadgeMenu_Badges = 17,
	BadgeMenu_Crests = 18,
	BadgeMenu_Suitcases = 19, // Unused in both PC and Arcade
	MainMenu_CustomMissions = 20,
	MT_Hunter = 21,
	MainMenu_CustomMissions2 = 22,
	MT_Hunter2 = 23,
	MissionSettings_Unk = 24, // FIXME
	CustomSquadSeries = 25,
	MissionSettings_Unk2 = 26, // FIXME
	GarageConnect = 27,
	WorldOfCarsConnect = 28,
	MainMenu_Garage = 29,
	GarageDetails = 30,
	WorldOfCars = 31, // Causes a game crash!
	WaitingForChallengers = 32, // FIXME
	UnkSubMenuGears2 = 33, // FIXME
	UnkSubMenuGears3 = 34, // FIXME
	CarSelect = 35,
};

enum class ControllerButton {
	Invalid,
	Left,
	Down,
	Right,
	Up,
	Square,
	Cross,
	Circle,
	Triangle,
	L1,
	R1,
	L2,
	R2,
	Start,
	Select,
	Stick1,
	Stick2,
	Back,
	Escape,
	Unk1,
	Unk2,
	Unk3,
	Unk4,
	Unk5,
	Unk6,
	Max
};

inline std::uintptr_t* g_InputPtr = reinterpret_cast<std::uintptr_t*>(0x018d3244);
inline std::uintptr_t* g_PopupCallback = reinterpret_cast<std::uintptr_t*>(0x01929b60);
inline void** g_PersistentData = reinterpret_cast<void**>(0x01926ef8);
inline void** g_Game = reinterpret_cast<void**>(0x01929bfc);

void __fastcall InitClearanceLevelData(std::uintptr_t unk) {
	auto iVar2 = FUN_00f675d0(*reinterpret_cast<void**>(0x0192c5ec), "Stat_SPY_POINTS", 1);
	auto local_c = FUN_00f66d60(*reinterpret_cast<void**>(0x0192c5ec), iVar2);
	iVar2 = FUN_00f675d0(*reinterpret_cast<void**>(0x0192c5ec), "Stat_CLEARANCE_LEVEL", 1);
	auto local_8 = FUN_00f66d60(*reinterpret_cast<void**>(0x0192c5ec), iVar2);

	int local_14 = 0, local_18 = 0;
	UnkExcelDataBase_GetUnk3(*reinterpret_cast<void**>(0x018ae110), local_8, &local_14, &local_18);
	float local_10 = static_cast<float>(local_c - local_14) / static_cast<float>(local_18 - local_14);
	if (0.0 <= local_10) {
		if (1.0 < local_10) {
			local_10 = 1.0;
		}
	}
	else {
		local_10 = 0.0;
	}
	if (static_cast<int>(local_8) < 0) {
		local_8 = 0;
	}
	else if (6 < static_cast<int>(local_8)) {
		local_8 = 6;
	}

	Flash_Movie_CallFlashFunction(*reinterpret_cast<std::uintptr_t*>(unk), "SetClearanceLevelData", 0, static_cast<double>(static_cast<int>(local_8)), static_cast<double>(local_10), static_cast<double>(local_c));
}

DefineInlineHook(SetInitialScreenState) {
	static void __cdecl callback(sunset::InlineCtx & ctx) {
		void* _this = *reinterpret_cast<void**>(ctx.ebp.unsigned_integer - 0x194);
		_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::TitleMenu, nullptr, true);
	}
};

DefineReplacementHook(CarsFrontEnd_SetScreen) {
	static void __fastcall callback(void* _this, uintptr_t edx, int unk, char* unk_name, unsigned char unk2) {
		if (unk_name != nullptr) {
			logger::log_format("[CarsFrontEnd::SetScreen] {}, {}, {}", unk, unk_name, unk2);
		}
		else {
			logger::log_format("[CarsFrontEnd::SetScreen] {}, nullptr, {}", unk, unk2);
		}
		original(_this, edx, unk, unk_name, unk2);
	}
};

DefineReplacementHook(CarsFrontEnd_GoBack) {
	static void __fastcall callback(void* _this) {
		if (IS_PC) {
			auto value = *reinterpret_cast<int*>(*reinterpret_cast<std::uintptr_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0x420) + 8);
			logger::log_format("[CarsFrontEnd::GoBack] Going back to {}...", value);
		}
		else {
			auto value = *reinterpret_cast<int*>(*reinterpret_cast<std::uintptr_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0x3FC) + 8);
			logger::log_format("[CarsFrontEnd::GoBack] Going back to {}...", value);
		}
		original(_this);
	}
};

DefineReplacementHook(RegisterGoBack) {
	static void __fastcall callback(std::uintptr_t _this) {
		GameSpecificFlashImpl* impl = reinterpret_cast<GameSpecificFlashImpl*>(_this + 4);
		GameSpecificFlashFunction* _callback = reinterpret_cast<GameSpecificFlashFunction*>((*reinterpret_cast<uintptr_t*>(_this + 0x3C)) + 0x258);
		impl->SetFlashVariableFunc("GoBack", SetFlashVariDef::ArgumentType::Null, _callback);
		original(_this);
	}
};

int GetNumUnlockedControllers() {
	int unlocked_controller_count = 0;
	for (int i = 0; i < 11; i = i + 1) {
		void* controller = WindowsSystemInputDriver_GetUnlockedController(*g_InputPtr, i);
		if (controller != nullptr) {
			std::uintptr_t* controller_inst = *reinterpret_cast<std::uintptr_t**>(controller);
			auto func = *reinterpret_cast<bool(__thiscall**)(void*)>(*controller_inst + 0x10);
			if (func(controller)) {
				unlocked_controller_count = unlocked_controller_count + 1;
			}
		}
	}
	return unlocked_controller_count;
}

DefineReplacementHook(OnConfirmHook) {
	static void __fastcall callback(void* _this, std::uintptr_t edx, char* _selected_menu, std::uintptr_t unk_menu) {
		std::string selected_menu = _selected_menu;
		logger::log_format("[CarsFrontEnd::OnConfirm] {}", selected_menu);
		*(reinterpret_cast<std::int32_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0x7CC)) += 1;
		if (std::fabs(std::fmod(static_cast<double>(*reinterpret_cast<std::int32_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0x7CC)), 3.0)) < 1E-07) {
			*(reinterpret_cast<const char**>(reinterpret_cast<std::uintptr_t>(_this) + 0x598)) = "Click_Select";
		}
		bool should_set_screen = false;

		std::int32_t menu_state = *(reinterpret_cast<std::int32_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0xA8));
		switch (menu_state) {
		case ExitToWindows:
			*reinterpret_cast<std::uint8_t*>(g_PopupCallback + 0x131) = 1;
			if (selected_menu == "SharedText_Yes") {
				std::exit(0);
			} else {
				_CarsFrontEnd_SetScreen(_this, MT_FrontEnd, nullptr, false);
				std::uint32_t array[2] = { 1, 0 };
				FUN_00ba0870(reinterpret_cast<std::uintptr_t>(_this) + 0x3e8, reinterpret_cast<std::uintptr_t>(&array));
			}
			break;
		case AutoSaveWarning:
			_CarsFrontEnd_SetScreen(_this, TitleMenu, nullptr, true);
			break;

		case TitleMenu:
			// FIXME: PC does a whole lot more with the global input driver, that we do not.
			WindowsSystemInputDriver_LockPlayerToController(*g_InputPtr, 0, 0); // g_InputPtr->LockPlayerToController(0, 0);
			// WindowsSystemInputDriver_LockPlayerToController(*g_InputPtr, 1, 1); // g_InputPtr->LockPlayerToController(1, 1);
			*reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(_this) + 0x7DC) = 0; // this->controller_index[0] = 0;
			// *reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(_this) + 0x7E0) = 1; // this->controller_index[1] = 1;
			_CarsFrontEnd_SetScreen(_this, SaveFileLoading, nullptr, true);
			*reinterpret_cast<bool*>(*g_PopupCallback + 0x10C) = true;
			break;
		case SaveSlots:
			{
				int index = std::atoi(_selected_menu);
				if (index < 0 || index > 2) {
					index = 0;
				}
				std::uintptr_t g_SaveGame = *reinterpret_cast<std::uintptr_t*>(0x0192b8d0);
				*reinterpret_cast<int*>(g_SaveGame + 0x60) = index;
				if (*reinterpret_cast<std::uint8_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0x4C + index * 24) == 0) {
					CSaveGame_ClearLoadedData(g_SaveGame);
					std::uintptr_t* g_GameSettings = *reinterpret_cast<std::uintptr_t**>(0x0192b8a8);
					auto func = *reinterpret_cast<void(__thiscall**)(std::uintptr_t*)>(*g_GameSettings + 0xC);
					func(g_GameSettings);
				}
				else {
					CSaveGame_UNK_00ee99a0(g_SaveGame, false);
				}
				std::uintptr_t game_settings = *reinterpret_cast<std::uintptr_t*>(0x0192b8a8);
				std::uintptr_t unk = *reinterpret_cast<std::uintptr_t*>(game_settings + 0x8);
				GameCommon_SetPlayerMusicVolume(*g_Game, *reinterpret_cast<float*>(unk + 4), 0);
				GameCommon_SetPlayerSfxVolume(*g_Game, *reinterpret_cast<float*>(unk + 8), 0);
				GameCommon_SetPlayerDialogueVolume(*g_Game, *reinterpret_cast<float*>(unk + 12), 0);
				_CarsFrontEnd_SetScreen(_this, MT_FrontEnd, nullptr, true);
			}
			break;

		case MainMenu_Extras:
			if (selected_menu == "FE_EX_Options") {
				_CarsFrontEnd_SetScreen(_this, Extras_Options, _selected_menu, true);
			}
			else if (selected_menu == "FE_EX_EnterCode") {
				_CarsFrontEnd_SetScreen(_this, Extras_EnterCode, _selected_menu, true);
			}
			else if (selected_menu == "FE_EX_Credits") {
				_CarsFrontEnd_SetScreen(_this, Extras_Credits, nullptr, true);
			}
			else if (selected_menu == "FE_EX_Cheats") {
				_CarsFrontEnd_SetScreen(_this, Extras_Cheats, nullptr, true);
			}
			break;

		case MT_FrontEnd:
			if (selected_menu == "FE_MM_Garage") {
				_CarsFrontEnd_SetScreen(_this, MainMenu_Garage, _selected_menu, true);
			}
			else if (selected_menu == "FE_MM_StoryMissions") {
				_CarsFrontEnd_SetScreen(_this, MainMenu_StoryMissions, _selected_menu, true);
			}
			else if (selected_menu == "FE_MM_CustomMissions") {
				_CarsFrontEnd_SetScreen(_this, MainMenu_CustomMissions, _selected_menu, true);
			}
			else if (selected_menu == "FE_MM_Badges") {
				_CarsFrontEnd_SetScreen(_this, MainMenu_Badges, _selected_menu, true);
			}
			else if (selected_menu == "FE_MM_Extras") {
				_CarsFrontEnd_SetScreen(_this, MainMenu_Extras, _selected_menu, true);
			}
			if (*reinterpret_cast<std::uintptr_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0xE4) != 0) {
				InitClearanceLevelData(*reinterpret_cast<std::uintptr_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0xE4));
			}
			break;

		case MainMenu_StoryMissions:
		{
			if (selected_menu.size() > 15) {
				char number = selected_menu[15];
				if (number < '0' || number > '9') {
					*reinterpret_cast<std::int8_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0x424) = 0;
				}
				else {
					*reinterpret_cast<std::int8_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0x424) = number - '0';
				}
				_CarsFrontEnd_SetScreen(_this, MissionSelect, _selected_menu, true);
			}
			else {
				*reinterpret_cast<std::int8_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0x424) = 0;
				_CarsFrontEnd_SetScreen(_this, MissionSelect, _selected_menu, true);
			}
		}
		break;

		case MissionSelect:
		{
			int unk_format_var = 0;
			if (selected_menu[0] == 'S') {
				(*g_GameProgressionManager)->unk_index = 0;
			}
			else {
				(*g_GameProgressionManager)->unk_index = -1;
				unk_format_var = std::atoi(_selected_menu);
			}
			int8_t unk_story_mission_index = *reinterpret_cast<std::int8_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0x424);
			*reinterpret_cast<char**>(reinterpret_cast<std::uintptr_t>(_this) + 0x414) = (*g_GameProgressionManager)->FormatStoryMission(unk_story_mission_index, unk_format_var);
			_CarsFrontEnd_SetScreen(_this, MissionDetails, nullptr, true);
		}
		break;

		case MissionDetails:
			should_set_screen = true;
			break;

		case MainMenu_Badges:
		{
			const char* oct_name = nullptr;
			if (selected_menu[6] == 'B') {
				_CarsFrontEnd_SetScreen(_this, BadgeMenu_Badges, _selected_menu, true);
				oct_name = "ui/badges.oct";
			}
			else if (selected_menu[6] == 'C') {
				_CarsFrontEnd_SetScreen(_this, BadgeMenu_Crests, _selected_menu, true);
				oct_name = "ui/crests.oct";
			}
			else if (selected_menu[6] == 'S') {
				_CarsFrontEnd_SetScreen(_this, BadgeMenu_Suitcases, _selected_menu, true);
				oct_name = "ui/suitcases.oct";
			}
			else {
				_CarsFrontEnd_SetScreen(_this, BadgeMenu_Badges, _selected_menu, true);
				oct_name = "ui/badges.oct";
			}
			void* engine_texture_loader = *reinterpret_cast<void**>(0x0192e194);
			if (*reinterpret_cast<void**>(reinterpret_cast<std::uintptr_t>(_this) + 0xCC) == nullptr) {
				*reinterpret_cast<void**>(reinterpret_cast<std::uintptr_t>(_this) + 0xCC) = Flash_EngineTextureLoader_LoadTextureSet(engine_texture_loader, oct_name, false, 0);
			}
		}
		break;

		case BadgeMenu_Badges:
		case BadgeMenu_Crests:
			{
				char buffer[64] = {};
				Genie::String badge_id{ _selected_menu };
				Genie::String localization_label{};
				Genie::String description_localization_label{};
				Genie::String unk4{}; // No idea what this is, seems to contain the same contents as `description_localization_label`.
				float unk5 = 0.0f;
				float unk6 = 0.0f;
				UnkExcelDataBase_GetUnk(*reinterpret_cast<void**>(0x018ae110), &badge_id, &localization_label);
				UnkExcelDataBase_GetUnk1(*reinterpret_cast<void**>(0x018ae110), &badge_id, &description_localization_label);
				UnkExcelDataBase_GetUnk2(*reinterpret_cast<void**>(0x018ae110), &badge_id, &unk4, &unk5, &unk6);
				int local_94 = static_cast<int>(std::floor(static_cast<double>(unk5 * 10.0)));
				int local_4c = static_cast<int>(std::fmod(static_cast<double>(local_94), 10.0));
				if (unk5 < unk6) {
					if (local_4c == 0) {
						sprintf_s(buffer, "%d", static_cast<int>(unk5));
					}
					else {
						float local_2a4 = unk5;
						if (unk6 - unk5 <= 0.05) {
							local_2a4 = unk6 - 0.06;
						}
						sprintf_s(buffer, "%.1f", local_2a4);
					}
				}
				else {
					sprintf_s(buffer, "%d", static_cast<int>(unk6));
				}
				char description[512] = {};
				char* description_format = CTranslator_Translate(reinterpret_cast<void*>(0x0192674c), description_localization_label.data, true);
				sprintf_s(description, description_format, static_cast<int>(unk6));
				if (menu_state == BadgeMenu_Badges) {
					unk6 = -1.0;
				}
				if (unk6 < unk5) {
					unk5 = unk6;
				}
				Flash_Movie_CallFlashFunction(unk_menu, "SetBadgeInfo", 0, localization_label.data, description, static_cast<double>(unk5), static_cast<double>(unk6), buffer);
			}
			break;

		case MainMenu_CustomMissions:
			_CarsFrontEnd_SetGameModeIndex(_this, _selected_menu);
			if (selected_menu[6] == 'S') {
				_CarsFrontEnd_SetScreen(_this, CustomSquadSeries, _selected_menu, true);
			}
			else {
				(*g_GameProgressionManager)->unk_index = -1;
				_CarsFrontEnd_SetScreen(_this, MT_Hunter, _selected_menu, true);
			}
			break;
		case MT_Hunter:
		case MT_Hunter2:
			if (_selected_menu[0] == 'T') {
				_CarsFrontEnd_SetLevelAndUnk(_this, _selected_menu);
				_CarsFrontEnd_UnkHandleTrackLengthType(_this, _selected_menu);
			}
			else {
				_CarsFrontEnd_SetLevelAndUnk(_this, _selected_menu);
			}

			if (menu_state == MT_Hunter2) {
				_CarsFrontEnd_SetScreen(_this, MissionSettings_Unk, nullptr, true);
			}
			else {
				int ai_car_count = GameProgressionManager_GetAICarCount(*g_GameProgressionManager);
				GameProgressionManager_SetAICarCount(*g_GameProgressionManager, ai_car_count);
				_CarsFrontEnd_SetScreen(_this, MissionSettings_Unk2, nullptr, true);
			}
			break;
		case MainMenu_CustomMissions2:
			_CarsFrontEnd_SetGameModeIndex(_this, _selected_menu);
			_CarsFrontEnd_SetScreen(_this, MT_Hunter2, nullptr, true);
			break;

		case MissionSettings_Unk:
			{
				GameProgressionManager_FUN_004e8400(*g_GameProgressionManager, false);
				GameProgressionManager_FUN_004ebaf0(*g_GameProgressionManager, *(reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(_this) + 0x46C)));
				std::uintptr_t local_34 = *(reinterpret_cast<std::uintptr_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0x3FC));
				while (local_34 != 0 && *reinterpret_cast<int*>(local_34 + 8) != CustomSquadSeries) {
					FUN_00ef3a30(reinterpret_cast<std::uintptr_t>(_this) + 0x3E8);
					char* dest = *reinterpret_cast<char**>(reinterpret_cast<std::uintptr_t>(_this) + 0x408);
					local_34 = *(reinterpret_cast<std::uintptr_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0x3FC));
					char* suffix = strrchr(dest, '-');
					int prefix_len = -1;
					if (suffix != nullptr) {
						prefix_len = suffix - dest;
					}
					FUN_0060e7d0(reinterpret_cast<std::uintptr_t>(_this) + 0x408, prefix_len, *reinterpret_cast<std::uintptr_t*>(*reinterpret_cast<std::uintptr_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0x408) - 0xC) - prefix_len);
				}
				char* dest = *reinterpret_cast<char**>(reinterpret_cast<std::uintptr_t>(_this) + 0x408);
				char* suffix = strrchr(dest, '-');
				int prefix_len = -1;
				if (suffix != nullptr) {
					prefix_len = suffix - dest;
				}
				std::uintptr_t result[10] = {};
				void* ppvVar6 = (void*)FUN_0060ee20(reinterpret_cast<std::uintptr_t>(_this) + 0x408, &result, *reinterpret_cast<std::uintptr_t*>(*reinterpret_cast<std::uintptr_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0x408) - 0xC) - (prefix_len + 1));
				FUN_00613000(reinterpret_cast<std::uintptr_t>(_this) + 0x40C, ppvVar6);
				Genie_String_Destructor(&result);
				FUN_0060e7d0(reinterpret_cast<std::uintptr_t>(_this) + 0x408, prefix_len, *reinterpret_cast<std::uintptr_t*>(*reinterpret_cast<std::uintptr_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0x408) - 0xC) - prefix_len);
				FUN_00ef3a30(reinterpret_cast<std::uintptr_t>(_this) + 0x3E8);
				_CarsFrontEnd_SetScreen(_this, CustomSquadSeries, "FE_MT_SquadSeries", false);
			}
			break;

		case CustomSquadSeries:
			{
				int index = std::atoi(_selected_menu);
				*(reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(_this) + 0x46C)) = index;
				GameProgressionManager_FUN_004ebab0(*g_GameProgressionManager, index);
				_CarsFrontEnd_SetScreen(_this, MainMenu_CustomMissions2, nullptr, true);
			}
			break;

		case MissionSettings_Unk2:
			should_set_screen = true;
			GameProgressionManager_FUN_004e8400(*g_GameProgressionManager, false);
			break;

		case GarageConnect:
			if (_stricmp(_selected_menu, "DLC_Connect") == 0) {
				*(reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(_this) + 0x800)) = 1;
			}
			break;

		case WorldOfCarsConnect:
			break;

		case MainMenu_Garage:
			if (selected_menu.starts_with("CAR_")) {
				void* dest = (reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(_this) + 0x114));
				Genie_String_Assign(dest, _selected_menu);
				_CarsFrontEnd_SetScreen(_this, GarageDetails, nullptr, true);
			}
			*(reinterpret_cast<float*>(reinterpret_cast<std::uintptr_t>(_this) + 0x7A4)) = 16.0f; // FIXME
			*(reinterpret_cast<float*>(reinterpret_cast<std::uintptr_t>(_this) + 0x7A8)) = 8.0f; // FIXME
			break;

		default:
			std::uintptr_t movie = *(reinterpret_cast<std::uintptr_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0xbc));
			if (movie != 0) {
				Flash_Movie_CallFlashFunction(movie, "ItemSelected", 0);
				*(reinterpret_cast<std::uint8_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0xC0)) = 1;
			}
			break;
		}
		if (should_set_screen) {
			/*
			int locked_controllers = 0;
			for (std::size_t i = 0; i < 11; i++) {
				if (WindowsSystemInputDriver_IsControllerLocked(*g_InputPtr, i)) {
					locked_controllers += 1;
				}
			}
			PersistentData_SetGlobal(*g_PersistentData, "NumPlayers", locked_controllers);
			PersistentData_SetGlobal(*g_PersistentData, "MultiPlayer", locked_controllers > 1);
			PersistentData_SetGlobal(*g_PersistentData, "ScreenFormat", locked_controllers > 4 ? 1 : locked_controllers);
			*/
 			_CarsFrontEnd_SetScreen(_this, CarSelect, nullptr, true);
		}
		return;

	}
};

DefineReplacementHook(SetButtonLayout) {
	static void __fastcall callback(std::uintptr_t _this, std::uintptr_t edx, int scheme) {
		logger::log_format("[Flash::FlashControlMapper::SetButtonLayout] Scheme: {}, ButtonCount: {}", scheme, *reinterpret_cast<int*>(_this + 0xA8));
		original(_this, edx, scheme);
		// if (IS_MACHINE_DESKTOP) {
			int* button_map = *reinterpret_cast<int**>(_this + 0xB0);
			button_map[55] = 6;
		// }
	}
};

DefineReplacementHook(CallFlashVariableFuncHook) {
	static void __fastcall callback(void* _this, uintptr_t edx, struct Movie* movie, char* method, void* args) {
		logger::log_format("[GameSpecificFlashImpl::CallFlashVariableFunc] {}", method);
		original(_this, edx, movie, method, args);
	}
};

DefineReplacementHook(ExternalInterfaceHandler_Callback) {
	static void __fastcall callback(void* _this, uintptr_t edx, struct Movie* movie, char* method, void* args, unsigned int arg_count) {
		logger::log_format("[Flash::Movie::ExternalInterfaceHandler::Callback] {}", method);
		original(_this, edx, movie, method, args, arg_count);
	}
};

DefineReplacementHook(CallFlashFunction) {
	static void __fastcall callback(void* _this, uintptr_t edx, void** active_function, char* function_name, void* return_value, va_list * arg_list) {
		logger::log_format("[Flash::Movie::CallFlashFunction] {}", function_name);
		original(_this, edx, active_function, function_name, return_value, arg_list);
	}
};

DefineReplacementHook(ToggleStateFlag) {
	static void __fastcall callback(std::uintptr_t _this) {
		original(_this);
		*reinterpret_cast<bool*>(_this + 0x10C) = true;
	}
};

DefineReplacementHook(ToggleStateFlag2) {
	static void __fastcall callback(std::uintptr_t _this) {
		original(_this);
		*reinterpret_cast<bool*>(_this + 0x10C) = false;
	}
};

std::atomic<bool> FIRST_TICK = true;
DefineInlineHook(ForceInitializeLocalPlayerOnFirstTick) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		if (FIRST_TICK) {
			*reinterpret_cast<int*>(ctx.ebp.unsigned_integer - 0x10) = 1;
			FIRST_TICK = false;
		}
	}
};

DefineReplacementHook(ShouldPauseHook) {
	static bool _cdecl callback() {
		return *reinterpret_cast<bool*>(0x018743e0);
	}
};

DefineReplacementHook(SetPauseFlag0) {
	static void _fastcall callback(void* _this, std::uintptr_t edx, std::uintptr_t unk) {
		*reinterpret_cast<bool*>(0x018743e0) = false;
		original(_this, edx, unk);
	}
};

DefineReplacementHook(SetPauseFlag1) {
	static void _fastcall callback(void* _this) {
		*reinterpret_cast<bool*>(0x018743e0) = true;
		original(_this);
	}
};

DefineReplacementHook(GameCommonLoop_PauseMessage_HandleMessage) {
	static void __fastcall callback(std::uintptr_t _this, std::uintptr_t edx, int* data, std::uint32_t actor_handle, std::uint32_t posted) {
		if (*data != 1 && *data != 2) {
			if (*data == 3) {
				std::uintptr_t* inst = *reinterpret_cast<std::uintptr_t**>(_this + 0x14);
				auto func = *reinterpret_cast<void(__thiscall**)(void*, void*)>(*inst + 0x44);
				func(inst, data);
			}
			else {
				std::uintptr_t* inst = *reinterpret_cast<std::uintptr_t**>(_this + 0x14);
				auto func = *reinterpret_cast<void(__thiscall**)(void*, void*)>(*inst + 0x4C);
				func(inst, data);
			}
		}
	}
};

void __fastcall InterceptSetAutoManDrift(void* data, std::uintptr_t edx, const char* name, std::uint32_t default_value) {
	int value = PersistentData_GetGlobal(data, name);
	PersistentData_SetGlobal(data, name, (value == 1) ? 0 : 1);
}

DefineInlineHook(SetAutoManDriftLocalVarFix) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		if (*reinterpret_cast<bool*>(ctx.ebp.unsigned_integer - 0x21)) {
			ctx.edx.unsigned_integer = PersistentData_GetGlobal(*g_PersistentData, reinterpret_cast<const char*>(ctx.ebp.unsigned_integer - 0x6C));
		}
	}
};

DefineInlineHook(GetAutoManDriftFix) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		const char* label = reinterpret_cast<const char*>(ctx.ebp.unsigned_integer - 0x6C);
		if (_strnicmp(label, "AutoDrift_", 10) == 0) {
			int value = PersistentData_GetGlobal(*g_PersistentData, label);
			*reinterpret_cast<std::uint8_t*>(ctx.ebp.unsigned_integer - 0x22) = value;
		}
	}
};

DefineInlineHook(OnSaveLoaded) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		void* _this = *reinterpret_cast<void**>(ctx.ebp.unsigned_integer - 0x28);
		_CarsFrontEnd_UNK_004c3b70(_this);
		if (*reinterpret_cast<bool*>((*reinterpret_cast<std::uintptr_t*>(0x018d3470)) + 0x7C)) {
			_CarsFrontEnd_SetScreen(_this, SaveSlots, nullptr, false);
		}
		else {
		_CarsFrontEnd_SetScreen(_this, MT_FrontEnd, nullptr, false);
	}
	}
};

struct ArcadeSetting {
	unsigned int type;
	int value;
	char string_value[80];
	unsigned int value_offset;
	bool initialized;
	char* key;
};

static_assert(sizeof(ArcadeSetting) == 0x64);

/*
DefineInlineHook(ForceEnableNetAndSetMachineId) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		ArcadeSetting* settings = reinterpret_cast<ArcadeSetting*>(ctx.eax.unsigned_integer + 0xA80);
		// enable network
		settings[0xB].value = 1;
		if (IS_MACHINE_DESKTOP) {
			// Set machine id to 1 if on desktop
			settings[0xC].value = 1;
		}
	}
};
*/

DefineInlineHook(OverrideDefaultVolume) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		*reinterpret_cast<std::uint32_t*>(ctx.ecx.unsigned_integer + 0x4) = 0x3F800000;
		*reinterpret_cast<std::uint32_t*>(ctx.ecx.unsigned_integer + 0x8) = 0x3F800000;
		*reinterpret_cast<std::uint32_t*>(ctx.ecx.unsigned_integer + 0xC) = 0x3F800000;
	}
};

DefineInlineHook(FixRestartVolume) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		// Fix the stack pointer since we nop'd a call to a __thiscall function.
		ctx.esp.signed_integer += 0xc;
		*reinterpret_cast<std::uint32_t*>(ctx.ebp.unsigned_integer - 0x4) = 0x3F800000;
	}
};

DefineReplacementHook(HandleSpinOut) {
	static void __fastcall callback(std::uintptr_t _this, std::uintptr_t edx, std::uint32_t a, std::uint32_t b, std::uint32_t c) {
		CarsVehicle* vehicle = *reinterpret_cast<CarsVehicle**>(_this + 4);
		*reinterpret_cast<std::uint8_t*>(*reinterpret_cast<std::uintptr_t*>(reinterpret_cast<std::uintptr_t>(vehicle) + 0x1470) + 0x1BC) = 1;
		if (vehicle->GetActiveMoves() != nullptr) {
			bool should_set = false;
			if (vehicle->GetActiveMoves()->m_actionState == ActiveMoves::ActionState::TwoWheelLeft || vehicle->GetActiveMoves()->m_actionState == ActiveMoves::ActionState::TwoWheelRight) {
				should_set = true;
			}
			if (should_set) {
				vehicle->GetActiveMoves()->SetTwoWheelingLeft(false);
				vehicle->GetActiveMoves()->SetTwoWheelingRight(false);
			}
		}
	}
};

DefineReplacementHook(LockInBro) {
	static bool __fastcall callback(CarsVehicle* _this, std::uintptr_t edx, bool param_1) {
		bool bVar1 = false;
		if (!_this->GetInTheZone()) {
			if (_this->GetActiveMoves() == nullptr) {
				bVar1 = false;
			}
			else {
				if (!param_1) {
					if (_this->GetActiveMoves()->m_turboing == false) {
						return false;
					}
					if (_this->GetActiveMoves()->m_turboTime > 0.5) {
						return false;
					}
				}
				float local_8 = *reinterpret_cast<float*>(*reinterpret_cast<std::uintptr_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0x117C) + 0x58);
				if (!param_1) {
					local_8 = local_8 - *reinterpret_cast<float*>(*reinterpret_cast<std::uintptr_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0x117C) + 0x54);
				}
				if (local_8 <= _this->GetCarEnergy().m_energy) {
					bVar1 = _this->GoInTheZone(false, false);
				}
				else {
					bVar1 = false;
				}
			}
		}
		else {
			bVar1 = false;
		}
		return bVar1;
	}
};

DefineReplacementHook(SideBashHandler) {
	static bool __fastcall callback(ActiveMoves* _this, std::uintptr_t edx, ActorHandle victim, bool bash_tie) {
		logger::log_format("[ActiveMoves::TriggerSideBashReactions] Attempting to handle sidebash reaction...");
		TriggerSideBashReactions(_this, edx, victim, bash_tie);
		return true;
	}
};

DefineReplacementHook(StartInvulnHook) {
	static void __fastcall callback(std::uintptr_t _this) {
		if (!*reinterpret_cast<bool*>(_this + 0x170)) {
			*reinterpret_cast<std::uint32_t*>(_this + 0x260) = 0x40200000;
		}
		else {
			*reinterpret_cast<bool*>(_this + 0x170) = 0;
		}
	}
};

DefineInlineHook(BeginFrameHook) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		*reinterpret_cast<bool*>(ctx.edx.unsigned_integer + 0x170) = false;
	}
};

DefineReplacementHook(FixRevITUI) {
	static void __fastcall callback(std::uintptr_t _this) {
		if (*reinterpret_cast<std::int32_t*>(_this + 0x40) == 0) {
			FUN_0116d4d0(*reinterpret_cast<std::uintptr_t*>(0x0192e19c), _this + 0x8cc, "to_energyMeter");
		}
		else {
			FUN_0116d4d0(*reinterpret_cast<std::uintptr_t*>(0x0192e19c), _this + 0x28, "init");
		}
		*reinterpret_cast<std::uint32_t*>(_this + 0x40) = 1;
	}
};

DefineInlineHook(InitHudElements) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		GameProgressionManager::MissionMode mode = *reinterpret_cast<GameProgressionManager::MissionMode*>(ctx.ebp.unsigned_integer - 0x2c);
		bool multiplayer = (*reinterpret_cast<std::uint32_t*>(0x018aa724) & 0x6000) != 0;
		int* local_8 = reinterpret_cast<int*>(ctx.ebp.unsigned_integer - 0x4);

		// NOTE: Theres a good chance that this fails horrifically in "multiplayer". Hence why we std::abort.
		if (multiplayer) {
			logger::log_format("[ArcadeEssentials::InitHudElements] Woah! How are you playing multiplayer?! Aborting...");
			std::abort();
		}

		if (mode == GameProgressionManager::MissionMode::Bomb || mode == GameProgressionManager::MissionMode::Hunter || mode == GameProgressionManager::MissionMode::Arena) {
			*local_8 = 7;
			*reinterpret_cast<int*>(ctx.esp.unsigned_integer) = 15;
		}
		else if (mode == GameProgressionManager::MissionMode::Collect) {
			*local_8 = 4;
			*reinterpret_cast<int*>(ctx.esp.unsigned_integer) = 10;
		}
		else if (mode == GameProgressionManager::MissionMode::Takedown) {
			*local_8 = 3;
			*reinterpret_cast<int*>(ctx.esp.unsigned_integer) = 5;
		}
		else if (mode == GameProgressionManager::MissionMode::Tutorial) {
			*local_8 = 1;
			*reinterpret_cast<int*>(ctx.esp.unsigned_integer) = 7;
		}
	}
};

DefineInlineHook(SupplyFlashArgs) {
	static void __cdecl callback(sunset::InlineCtx & ctx) {
		*reinterpret_cast<std::uintptr_t*>(ctx.esp.unsigned_integer) = ctx.ebp.unsigned_integer - 0xE8;
	}
};

DefineInlineHook(SupplyFlashArgs2) {
	static void __cdecl callback(sunset::InlineCtx & ctx) {
		*reinterpret_cast<std::uintptr_t*>(ctx.esp.unsigned_integer) = ctx.ebp.unsigned_integer - 0x30;
	}
};

DefineInlineHook(FixTurboUI) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		std::uintptr_t* inst = *reinterpret_cast<std::uintptr_t**>(ctx.ebp.unsigned_integer - 0x18);
		Genie::String* button_turbo = reinterpret_cast<Genie::String*>(ctx.ebp.unsigned_integer - 0x3C);
		auto func = *reinterpret_cast<void(__thiscall**)(void*, Genie::String*, const char*, const char*)>(*inst + 0x50);
		func(inst, button_turbo, "turbo", "normal");
	}
};

DefineInlineHook(SRandHook) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		ctx.eax.unsigned_integer = 0;
	}
};

extern "C" void __stdcall Pentane_Main() {
	/*
	unsigned long computer_name_len = 1024;
	wchar_t computer_name[1024];
	GetComputerNameExW(ComputerNameDnsHostname, computer_name, &computer_name_len);
	if (std::wstring_view(computer_name) == L"DESKTOP-CRU7H46") {
		IS_MACHINE_DESKTOP = true;
	}
	*/

	// FIXME: link against Pentane.lib properly instead of this bullshit!!!!
	Pentane_LogUTF8 = reinterpret_cast<void(*)(PentaneCStringView*)>(GetProcAddress(GetModuleHandleA("Pentane.dll"), "Pentane_LogUTF8"));
	wchar_t exe_name[1024];
	GetModuleFileNameW(nullptr, exe_name, 1024);
	PathStripPathW(exe_name);
	if (!std::wstring_view(exe_name).contains(L"daemon")) {
		IS_PC = true;
	}
	if (IS_PC) {
		logger::log("[ArcadeEssentials::Pentane_Main] WARN: Arcade executable not detected! Assuming Cars 2: The Video Game (PC)...");
		/* DEBUGGING HOOKS START */
		// ExternalInterfaceHandler_Callback::install_at_ptr(0x010dc930);
		// CallFlashFunction::install_at_ptr(0x010dae50);
		// CarsFrontEnd_SetScreen::install_at_ptr(0x0048c910);
		// CarsFrontEnd_GoBack::install_at_ptr(0x00489af0);
		/* DEBUGGING HOOKS END */
	}
	else {
		/* DEBUGGING HOOKS START */
		// ExternalInterfaceHandler_Callback::install_at_ptr(0x0116a080);
		// CallFlashFunction::install_at_ptr(0x01168710);
		// CarsFrontEnd_SetScreen::install_at_ptr(0x004c1440);
		// CarsFrontEnd_GoBack::install_at_ptr(0x004be200);
		/* DEBUGGING HOOKS END */

		// Set's ArcadeManager's initial VideoState to 16 (GAME_START), in order to force the game to skip all intro cutscenes.
		sunset::utils::set_permission(reinterpret_cast<void*>(0x004512ad), 1, sunset::utils::Perm::ExecuteReadWrite);
		*reinterpret_cast<char*>(0x004512ad) = 16;

		// Replaces a call to CMessageDispatcher::SendMessageToAll that shuts off the Globe for a call to CarsFrontEnd::SetScreen to allow the game to boot directly to the title screen.
		sunset::inst::nop(reinterpret_cast<void*>(0x004ba81c), 19);
		SetInitialScreenState::install_at_ptr(0x004ba81c);

		static const char GLOBE_ON[] = "GlobeOn\0";
		// Removes a call to CMessageDispatcher::SendMessageToAll from a CarsFrontEnd member function that shuts off the Globe on the title screen.
		sunset::inst::nop(reinterpret_cast<void*>(0x004ba108), 19);
		// Swaps out the "GlobeOff" string for "GlobeOn" in a call to CMessageDispatcher::SendMessageToAll from within the title screen case inside CarsFrontEnd::SetScreen.
		sunset::inst::push_u32(reinterpret_cast<void*>(0x004c1c6e), reinterpret_cast<uintptr_t>(&GLOBE_ON));
		// Swaps out the "GlobeOff" string for "GlobeOn" in a call to CMessageDispatcher::SendMessageToAll.
		sunset::inst::push_u32(reinterpret_cast<void*>(0x004c39f6), reinterpret_cast<uintptr_t>(&GLOBE_ON));
		// No-ops the code for menu state UnkMenuEmpty that forces the machine into ExitToWindows.
		sunset::inst::nop(reinterpret_cast<void*>(0x004c170f), 14);

		// Registers the otherwise-missing `GoBack` callback inside CarsFrontEndFlashCallbacks::FrontendFlashFunctions::SetupFlashFunctions.
		RegisterGoBack::install_at_ptr(0x004c93e0);
		
		// Implements most of the logic for transitioning from screen to screen.
		OnConfirmHook::install_at_ptr(0x004be010);
		
		// Allows the game to transition from SaveFileLoading to MT_FrontEnd, as well as reset the volume to the user-configured values.
		sunset::inst::nop(reinterpret_cast<void*>(0x004c3b33), 0x33);
		OnSaveLoaded::install_at_ptr(0x004c3b33);
		
		// Forcibly maps the A/Cross button to 55, allowing menu navigation with A/Cross.
		SetButtonLayout::install_at_ptr(0x01163f70);

		// Redirects the game's WinArcadeInputDriver to the otherwise-unused WindowsSystemInputDriver (likely a leftover from the PC port).
		sunset::inst::push_u32(reinterpret_cast<void*>(0x0080cf64), 0x500); // Patch argument to operator.new
		sunset::inst::call(reinterpret_cast<void*>(0x0080cf8d), reinterpret_cast<void*>(0x00814fa0)); // Replace call to constructor
		sunset::inst::call(reinterpret_cast<void*>(0x0080da31), reinterpret_cast<void*>(0x008156f0)); // Replace call to ::Initialize member function
		
		// No-ops code that floods the logger with "dpad pressed" messages.
		sunset::inst::nop(reinterpret_cast<void*>(0x00814a55), 0x3D);

		// Stubs the function that otherwise triggers a a system reboot (What the actual fuck RT??)
		sunset::utils::set_permission(reinterpret_cast<void*>(0x00458680), 1, sunset::utils::Perm::ExecuteReadWrite);
		*reinterpret_cast<std::uint8_t*>(0x00458680) = 0xC3;

		// Stubs the function that schedules a maintenance reboot when the framerate dips below 30.
		// (looking at u max)
		sunset::utils::set_permission(reinterpret_cast<void*>(0x00455650), 1, sunset::utils::Perm::ExecuteReadWrite);
		*reinterpret_cast<std::uint8_t*>(0x00455650) = 0xC3;
			
		// Disables AutoPilot
		sunset::inst::nop(reinterpret_cast<void*>(0x004f3c67), 0x2A);
		
		// Disables "System going down for Maintenance" message.
		sunset::inst::nop(reinterpret_cast<void*>(0x004530c9), 0xF);

		// Fixes an issue where ending an event sent you to the attract menus.
		sunset::inst::nop(reinterpret_cast<void*>(0x004fe25d), 0x66);
		sunset::inst::nop(reinterpret_cast<void*>(0x004fe2fa), 0x20);
		// Allows the StorageStateMachine to bring the user back to race select after finishing an event.
		ToggleStateFlag::install_at_ptr(0x00e9a5c0);
		// I'll be honest, I have no idea what this does. Hopefully it fixes something!
		ToggleStateFlag2::install_at_ptr(0x00e9a770);

		// Prevents the QR code image from being generated.
		sunset::utils::set_permission(reinterpret_cast<void*>(0x00466000), 1, sunset::utils::Perm::ExecuteReadWrite);
		*reinterpret_cast<std::uint32_t*>(0x00466000) = 0x00000CC2;

		// Force initialize the local player struct on the first call to ArcadeManager::UpdateFrontEnd by setting a local variable to 1.
		// As this expects the first tick to send the player straight to the title screen, this approach will NOT work if attract videos are re-enabled.
		ForceInitializeLocalPlayerOnFirstTick::install_at_ptr(0x0045218b);

		// Instead of reading from the pause global variable like PC does, Arcade instead... checks the lower bits of g_Game? Whatever, this fixes that.
		ShouldPauseHook::install_at_ptr(0x00ecade0);
		// Patches two functions to properly update the pause flag.
		SetPauseFlag0::install_at_ptr(0x00551ab0);
		SetPauseFlag1::install_at_ptr(0x004fd0b0);
		// Allows the pause/unpause messages to properly propogate through the message dispatcher.
		GameCommonLoop_PauseMessage_HandleMessage::install_at_ptr(0x00eb66d0);

		// Forces the AutoDrift_ PersistentData globals to toggle between 1 and 0 instead of being forced to 1 no matter what.
		sunset::inst::call(reinterpret_cast<void*>(0x004cd3e6), InterceptSetAutoManDrift);
		// Updates a local variable with the last set PersistentData value for AutoDrift_.
		SetAutoManDriftLocalVarFix::install_at_ptr(0x004cd3fc);
		// Fixes an issue where the UI doesn't update until the second X press.
		GetAutoManDriftFix::install_at_ptr(0x004cd4f7);

		// Un-stubs the function responsible for initializing the clearance level data and passing it over to Flash.
		sunset::inst::jmp(reinterpret_cast<void*>(0x004b8790), InitClearanceLevelData);

		// Prevents the game from using ArcadeData's master volume on restarting an event.
		sunset::inst::nop(reinterpret_cast<void*>(0x004f75a7), 5);
		FixRestartVolume::install_at_ptr(0x004f75a7);
		// Completely stub ArcadeManager::SetVolume.
		sunset::utils::set_permission(reinterpret_cast<void*>(0x004515e0), 4, sunset::utils::Perm::ReadWrite);
		*reinterpret_cast<std::uint32_t*>(0x004515e0) = 0x900004C2;
		// Overrides the default volumes used when there is no save file with the maximum values.
		OverrideDefaultVolume::install_at_ptr(0x00e9b89b);

		// Restores the ability for cars to respond to being sidebashed.
		SideBashHandler::install_at_ptr(0x006fac40);

		// Allows the user to backwards-drive for as long as they want.
		sunset::inst::nop(reinterpret_cast<void*>(0x006f73eb), 4);
		// Prevents RaceManager from trying to inject ControllerButton::Triangle presses in the input driver.
		sunset::inst::nop(reinterpret_cast<void*>(0x004f3ecf), 2);
		
		// Prevents events from ending once the timer hits 180s.
		sunset::utils::set_permission(reinterpret_cast<void*>(0x004F3B79), 1, sunset::utils::Perm::ExecuteReadWrite);
		*reinterpret_cast<std::uint8_t*>(0x004F3B79) = 0;

		// Restores the ability for cars to spin out.
		HandleSpinOut::install_at_ptr(0x006c3740);
		sunset::inst::jmp(reinterpret_cast<void*>(0x006d73a0), HandleWipeout);
		sunset::inst::jmp(reinterpret_cast<void*>(0x006d73b0), HandleCommenceWipeout);
		
		// Brings the ITZ initialization logic closer to the original game.
		LockInBro::install_at_ptr(0x006b3320);
		// Prevents the game from forcing ITZ on every turbo.
		sunset::utils::set_permission(reinterpret_cast<void*>(0x006F96B2), 5, sunset::utils::Perm::ExecuteReadWrite);
		*reinterpret_cast<std::uint32_t*>(0x006F96B2) = 0x9004C483;
		*reinterpret_cast<std::uint8_t*>(0x006F96B6) = 0x90;
		sunset::inst::nop(reinterpret_cast<void**>(0x006f96bc), 6);

		// Fixes an issue where lemon cars would remain indefinetly `Invulnerable`.
		BeginFrameHook::install_at_ptr(0x006d432c);
		StartInvulnHook::install_at_ptr(0x006d5640);
		sunset::utils::set_permission(reinterpret_cast<void*>(0x006d3898), 1, sunset::utils::Perm::ExecuteReadWrite);
		*reinterpret_cast<std::uint8_t*>(0x006d3898) = 0;
		
		// Fixes an issue where non-race mission modes would always fall back to the same UI.
		InitHudElements::install_at_ptr(0x0055114c);

		// Fixes the Rev-It! icon meter dissappearing after an event starts.
		FixRevITUI::install_at_ptr(0x00547ca0);
		static const char DISPLAY_BUTTON_PROMPT[] = "DisplayButtonPrompt";
		sunset::inst::push_u32(reinterpret_cast<void*>(0x0054528b), reinterpret_cast<std::uintptr_t>(&DISPLAY_BUTTON_PROMPT));
		sunset::inst::push_u32(reinterpret_cast<void*>(0x005454f6), reinterpret_cast<std::uintptr_t>(&DISPLAY_BUTTON_PROMPT));
		SupplyFlashArgs::install_at_ptr(0x0054528b);
		SupplyFlashArgs2::install_at_ptr(0x005454f6);
		sunset::utils::set_permission(reinterpret_cast<void*>(0x00545288), 1, sunset::utils::Perm::ExecuteReadWrite);
		*reinterpret_cast<std::uint8_t*>(0x00545288) = 3;
		sunset::utils::set_permission(reinterpret_cast<void*>(0x005454f3), 1, sunset::utils::Perm::ExecuteReadWrite);
		*reinterpret_cast<std::uint8_t*>(0x005454f3) = 3;
		// Restores the TURBO meter/icon.
		FixTurboUI::install_at_ptr(0x0068fb5b);

		// Removes a premature `jmp` that otherwise ranks AI sidestep actions as 0.0f no matter what. (WTF Microsoft? How is this code still there past the return? Whatever, thanks I guess...)
		sunset::inst::nop(reinterpret_cast<void*>(0x004215c9), 11);

#ifdef _DEBUG
		// Sets the random seed to zero instead of the system time.
		SRandHook::install_at_ptr(0x00450c48);
#endif
		// ForceEnableNetAndSetMachineId::install_at_ptr(0x00450791);
		logger::log("[ArcadeEssentials::Pentane_Main] Installed hooks!");
	}
}