#include <iostream>
#include <fstream>
#include <array>
#include <filesystem>
#include <unordered_map>
#include <Windows.h>
#include <shlwapi.h>
#include <sunset.hpp>
#include <d3d9.h>
#include <dwmapi.h>
#include "pentane.hpp"
#include "config.hpp"
#include "Game/MiscOffsets.hpp"
#include "Game/GameSpecificFlashImpl.hpp"
#include "Game/Genie/String.hpp"
#include "Game/GameProgressionManager.hpp"
#include "Game/Stage/StageEntity.hpp"
#include "Game/Components/ActiveMoves.hpp"
#include "Game/Components/CarsReactionMonitor.hpp"
#include "Game/Components/CarsWeaponInventory.hpp"
#include "Game/Stage/Cars2VehicleDBlock.hpp"
#include "Patch/Debug/CMessageLogging.hpp"
#include "Patch/Input/KeyControllerInputDriver.hpp"
#include "Patch/Input/WindowsSystemInputDriver.hpp"
// #include "Patch/Input/WindowsControllerInputDriver.hpp"
// #include "Patch/Input/XInputInputDriver.hpp"
#include "Patch/MultiPlayer.hpp"
#include "Patch/OptionFlashCallbacks.hpp"
#include "Patch/FMV/Driver.hpp"
#include "Game/Renderer/DataTypes.hpp"
#include "Game/FrontEnd/CarsFrontEnd.hpp"
#include "Game/Avatar.hpp"
#include "Game/GamePlay/Race/RaceManager.hpp"
#include "Game/Scaleform/Wrapper/Movie.hpp"
#include "Game/Scaleform/Wrapper/FlashControlMapper.hpp"
#include "Game/FMV/Driver.hpp"
#include "Game/FMV/Player.hpp"
#include "Game/FMV/Subtitles.hpp"
#include "Game/AudioEventManager.hpp"
#include "Axel/Axel.hpp"
#include "Game/Hud/CarsHud.hpp"

#ifdef MP_STRATEGY_AXEL
#pragma comment(lib, "steam_api.lib")
#pragma comment(lib, "sdkencryptedappticket.lib")
#endif

void __fastcall init_clearance_level_data(std::uintptr_t unk) {
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
#ifdef _DEBUG
		if (unk_name != nullptr) {
			logger::log_format("[CarsFrontEnd::SetScreen] {}, {}, {}", unk, unk_name, unk2);
		}
		else {
			logger::log_format("[CarsFrontEnd::SetScreen] {}, nullptr, {}", unk, unk2);
		}
#endif
		CarsFrontEndScreen menu_state = *(reinterpret_cast<CarsFrontEndScreen*>(reinterpret_cast<std::uintptr_t>(_this) + 0xA8));
		if (menu_state == CarsFrontEndScreen::SaveSlots) {
			std::uintptr_t* mini_menu = *(reinterpret_cast<std::uintptr_t**>(reinterpret_cast<std::uintptr_t>(_this) + 0xe4));
			if (mini_menu != nullptr) {
				if (*mini_menu != 0) {
					logger::log_format("[CarsFrontEnd::SetScreen] Removing last button.");
					Flash_Movie_CallFlashFunction(*mini_menu, "RemoveLastButton", 0);
				}
			}
		}
		original(_this, edx, unk, unk_name, unk2);
	}
};

DefineReplacementHook(CarsFrontEnd_GoBack) {
	static void __fastcall callback(CarsFrontEnd* _this) {
		auto value = *reinterpret_cast<int*>(*reinterpret_cast<std::uintptr_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0x3FC) + 8);
#ifdef _DEBUG
		logger::log_format("[CarsFrontEnd::GoBack] Going back to {}...", value);
#endif
#ifdef MP_STRATEGY_AXEL
		axel::ui::on_retract(_this);
#endif
		original(_this);
	}
};

DefineReplacementHook(RegisterMissingFlashFuncs) {
	static void __fastcall callback(std::uintptr_t _this) {
		GameSpecificFlashImpl* impl = reinterpret_cast<GameSpecificFlashImpl*>(_this + 4);
		GameSpecificFlashFunction* _callback = reinterpret_cast<GameSpecificFlashFunction*>((*reinterpret_cast<uintptr_t*>(_this + 0x3C)) + 0x258);
		impl->SetFlashVariableFunc("GoBack", SetFlashVariDef::ArgumentType::Null, _callback);
		_callback = reinterpret_cast<GameSpecificFlashFunction*>((*reinterpret_cast<uintptr_t*>(_this + 0x3C)) + 0x1F8);
		impl->SetFlashVariableFunc("GetSpyPoints", SetFlashVariDef::ArgumentType::Number, _callback);
		original(_this);
	}
};

int get_num_unlocked_controllers() {
	int unlocked_controller_count = 0;
	for (int i = 0; i < 11; i = i + 1) {
		void* controller = (*g_InputPtr)->GetUnlockedController(i);
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
	static void __fastcall callback(CarsFrontEnd* _this, std::uintptr_t edx, char* _selected_menu, std::uintptr_t unk_menu) {
		std::string selected_menu = _selected_menu;
		logger::log_format("[CarsFrontEnd::OnConfirm] {}", selected_menu);
		*(reinterpret_cast<std::int32_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0x7CC)) += 1;
		if (std::fabs(std::fmod(static_cast<double>(*reinterpret_cast<std::int32_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0x7CC)), 3.0)) < 1E-07) {
			*(reinterpret_cast<const char**>(reinterpret_cast<std::uintptr_t>(_this) + 0x598)) = "Click_Select";
		}
		bool should_set_screen = false;
		bool should_play_item_selected = true;

		CarsFrontEndScreen screen = *(reinterpret_cast<CarsFrontEndScreen*>(reinterpret_cast<std::uintptr_t>(_this) + 0xA8));
		switch (screen) {
		case CarsFrontEndScreen::ExitToTitleScreen:
			*reinterpret_cast<std::uint8_t*>(g_PopupCallback + 0x131) = 1;
			if (selected_menu == "SharedText_Yes") {
				/*
				if (((*g_PopupCallback) + 0xc8) == *reinterpret_cast<std::uintptr_t*>((*g_PopupCallback) + 0x4C)) {
					StorageStateMachine_PopupCallback_FUN_00e9b3c0(*g_PopupCallback);
					*reinterpret_cast<std::uintptr_t*>((*g_PopupCallback) + 0x48) = ((*g_PopupCallback) + 0x58);
				}
				_CarsFrontEnd_GoBack(_this);
				*/
				std::exit(0);
			}
			else {
				// `CarsFrontEnd::SetScreen` destroys a few PersistentData keys when you re-enter `MT_FrontEnd`, so we back them up before we call it and restore them right after.
				int savePlayersCreated = PersistentData_GetGlobal(*g_PersistentData, "PlayersCreated");
				int savePlayerCount = PersistentData_GetGlobal(*g_PersistentData, "NumPlayers");
				int saveMp = PersistentData_GetGlobal(*g_PersistentData, "MultiPlayer");
				int saveScreenFormat = PersistentData_GetGlobal(*g_PersistentData, "ScreenFormat");

				_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::MT_FrontEnd, nullptr, false);

				PersistentData_SetGlobal(*g_PersistentData, "PlayersCreated", savePlayersCreated);
				PersistentData_SetGlobal(*g_PersistentData, "NumPlayers", savePlayerCount);
				PersistentData_SetGlobal(*g_PersistentData, "MultiPlayer", saveMp);
				PersistentData_SetGlobal(*g_PersistentData, "ScreenFormat", saveScreenFormat);

				std::uint32_t array[2] = { 1, 0 };
				FUN_00ba0870(reinterpret_cast<std::uintptr_t>(_this) + 0x3e8, reinterpret_cast<std::uintptr_t>(&array));
			}
			should_play_item_selected = false;
			break;
		case CarsFrontEndScreen::AutoSaveWarning:
			_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::TitleMenu, nullptr, true);
			should_play_item_selected = false;
			break;

		case CarsFrontEndScreen::TitleMenu:
			{
			// For some god-forsaken reason, the game refuses to accept player 0 input from anything but controller 0. So we silently swap whoever pressed START at the title screen with controller 0.
			int start_controller = *_selected_menu - 0x30;
			if (start_controller != 0) {
				(*g_InputPtr)->SwapControllers(0, start_controller);
				start_controller = 0;
			}
			// If theres no controller locked to player 0, we want to lock player 0 to whoever pressed START at the title screen (due to the above hack this should always be zero anyway).
			if (!(*g_InputPtr)->ControllerLocked(0)) {
				(*g_InputPtr)->LockPlayerToController(0, start_controller);
				*reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(_this) + 0x7DC) = start_controller;
			}
			// reinterpret_cast<std::uint8_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0xOFFSET)) = 1;
			// reinterpret_cast<std::uint32_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0xOFFSET)) = CMasterTimer_GetOSTime();
			CSaveGame_UNK_00eea2e0(*g_SaveGame);
			CSaveGame_ClearLoadedData(*g_SaveGame);
			FUN_0080df70(*reinterpret_cast<std::uintptr_t*>(0x018d323c));
			*reinterpret_cast<bool*>(*g_PopupCallback + 0x10C) = true;
			_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::SaveFileLoading, nullptr, true);
			*reinterpret_cast<std::uint8_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0x424) = static_cast<std::uint8_t>(*reinterpret_cast<std::uint32_t*>(reinterpret_cast<std::uintptr_t>(*g_GameProgressionManager) + 0x108));
			should_play_item_selected = false;
			}
			break;
		case CarsFrontEndScreen::SaveSlots:
		{
			int index = std::atoi(_selected_menu);
			if (index < 0 || index > 2) {
				index = 0;
			}
			*reinterpret_cast<int*>(*g_SaveGame + 0x60) = index;
			if (*reinterpret_cast<std::uint8_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0x4C + index * 24) == 0) {
				CSaveGame_ClearLoadedData(*g_SaveGame);
				std::uintptr_t* g_GameSettings = *reinterpret_cast<std::uintptr_t**>(0x0192b8a8);
				auto func = *reinterpret_cast<void(__thiscall**)(std::uintptr_t*)>(*g_GameSettings + 0xC);
				func(g_GameSettings);
			}
			else {
				CSaveGame_UNK_00ee99a0(*g_SaveGame, false);
			}
			std::uintptr_t game_settings = *reinterpret_cast<std::uintptr_t*>(0x0192b8a8);
			std::uintptr_t unk = *reinterpret_cast<std::uintptr_t*>(game_settings + 0x8);
			GameCommon_SetPlayerMusicVolume(*g_Game, *reinterpret_cast<float*>(unk + 4), 0);
			GameCommon_SetPlayerSfxVolume(*g_Game, *reinterpret_cast<float*>(unk + 8), 0);
			GameCommon_SetPlayerDialogueVolume(*g_Game, *reinterpret_cast<float*>(unk + 12), 0);
			_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::MT_FrontEnd, nullptr, true);
			should_play_item_selected = false;
		}
		break;

		case CarsFrontEndScreen::MainMenu_Extras:
			if (selected_menu == "FE_EX_Options") {
				_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::Extras_Options, _selected_menu, true);
			}
			else if (selected_menu == "FE_EX_EnterCode") {
				_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::Extras_EnterCode, _selected_menu, true);
			}
			else if (selected_menu == "FE_EX_Credits") {
				_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::Extras_Credits, nullptr, true);
			}
			else if (selected_menu == "FE_EX_Cheats") {
				_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::Extras_Cheats, nullptr, true);
			}
			else if (selected_menu == "FE_EX_Graphics") {
				_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::Extras_Options, _selected_menu, true);
			}
			break;

		case CarsFrontEndScreen::MT_FrontEnd:
			if (selected_menu == "FE_MM_Garage") {
				_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::MainMenu_Garage, _selected_menu, true);
			}
			else if (selected_menu == "FE_MM_StoryMissions") {
				_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::MainMenu_StoryMissions, _selected_menu, true);
			}
			else if (selected_menu == "FE_MM_CustomMissions") {
				_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::MainMenu_CustomMissions, _selected_menu, true);
			}
			else if (selected_menu == "FE_MM_Badges") {
				_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::MainMenu_Badges, _selected_menu, true);
			}
			else if (selected_menu == "FE_MM_Extras") {
				_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::MainMenu_Extras, _selected_menu, true);
			}
			else if (selected_menu == "FE_MM_WOC") {
				_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::WorldOfCars_Missions, _selected_menu, true);
			}
			/*
			else if (selected_menu == "Win32Wii_Scn_ExitToWindows") {
				FUN_004d0250(reinterpret_cast<std::uintptr_t>(_this), 2);
				_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::MT_FrontEnd, nullptr, false);
			}
			*/
#ifdef MP_STRATEGY_AXEL
			else if (selected_menu == "FE_MM_Axel") {
				_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::Axel_Online, _selected_menu, true);
			}
#endif
			if (*reinterpret_cast<std::uintptr_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0xE4) != 0) {
				init_clearance_level_data(*reinterpret_cast<std::uintptr_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0xE4));
			}
			break;

		case CarsFrontEndScreen::MainMenu_StoryMissions:
		{
			if (selected_menu.size() > 15) {
				char number = selected_menu[15];
				if (number < '0' || number > '9') {
					*reinterpret_cast<std::int8_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0x424) = 0;
				}
				else {
					*reinterpret_cast<std::int8_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0x424) = number - '0';
				}
				_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::StoryMissions_MissionSelect, _selected_menu, true);
			}
			else {
				*reinterpret_cast<std::int8_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0x424) = 0;
				_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::StoryMissions_MissionSelect, _selected_menu, true);
			}
		}
		break;

		case CarsFrontEndScreen::StoryMissions_MissionSelect:
		{
			int unk_format_var = 0;
			if (selected_menu[0] == 'S') {
				(*g_GameProgressionManager)->m_unkIndex = 0;
			}
			else {
				(*g_GameProgressionManager)->m_unkIndex = -1;
				unk_format_var = std::atoi(_selected_menu);
			}
			int8_t unk_story_mission_index = *reinterpret_cast<std::int8_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0x424);
			*reinterpret_cast<char**>(reinterpret_cast<std::uintptr_t>(_this) + 0x414) = (*g_GameProgressionManager)->FormatStoryMission(unk_story_mission_index, unk_format_var);
			_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::StoryMissions_MissionDetails, nullptr, true);
		}
		break;

		case CarsFrontEndScreen::StoryMissions_MissionDetails:
			should_set_screen = true;
			break;

		case CarsFrontEndScreen::MainMenu_Badges:
		{
			const char* oct_name = nullptr;
			if (selected_menu[6] == 'B') {
				_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::BadgeMenu_Badges, _selected_menu, true);
				oct_name = "ui/badges.oct";
			}
			else if (selected_menu[6] == 'C') {
				_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::BadgeMenu_Crests, _selected_menu, true);
				oct_name = "ui/crests.oct";
			}
			else if (selected_menu[6] == 'S') {
				_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::BadgeMenu_Suitcases, _selected_menu, true);
				oct_name = "ui/suitcases.oct";
			}
			else {
				_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::BadgeMenu_Badges, _selected_menu, true);
				oct_name = "ui/badges.oct";
			}
			void* engine_texture_loader = *reinterpret_cast<void**>(0x0192e194);
			if (*reinterpret_cast<void**>(reinterpret_cast<std::uintptr_t>(_this) + 0xCC) == nullptr) {
				*reinterpret_cast<void**>(reinterpret_cast<std::uintptr_t>(_this) + 0xCC) = Flash_EngineTextureLoader_LoadTextureSet(engine_texture_loader, oct_name, false, 0);
			}
			should_play_item_selected = false;
		}
		break;

		case CarsFrontEndScreen::BadgeMenu_Badges:
		case CarsFrontEndScreen::BadgeMenu_Crests:
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
				if (screen == CarsFrontEndScreen::BadgeMenu_Badges) {
					unk6 = -1.0;
				}
				if (unk6 < unk5) {
					unk5 = unk6;
				}
				Flash_Movie_CallFlashFunction(unk_menu, "SetBadgeInfo", 0, localization_label.data, description, static_cast<double>(unk5), static_cast<double>(unk6), buffer);
				should_play_item_selected = false;
			}
			break;

		case CarsFrontEndScreen::MainMenu_CustomMissions:
			_CarsFrontEnd_SetGameModeIndex(_this, _selected_menu);
			if (selected_menu[6] == 'S') {
				_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::CustomSquadSeries, _selected_menu, true);
			}
			else {
				(*g_GameProgressionManager)->m_unkIndex = -1;
				_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::MainMenu_MissionSelect, _selected_menu, true);
			}
			break;
		case CarsFrontEndScreen::MainMenu_MissionSelect:
		case CarsFrontEndScreen::MainMenu_MissionSelect_SquadSeries:
			if (_selected_menu[0] == 'T') {
				_CarsFrontEnd_SetLevelAndUnk(_this, _selected_menu);
				_CarsFrontEnd_UnkHandleTrackLengthType(_this, _selected_menu);
			}
			else {
				_CarsFrontEnd_SetLevelAndUnk(_this, _selected_menu);
			}
			if (screen == CarsFrontEndScreen::MainMenu_MissionSelect_SquadSeries) {
				_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::MissionSettings_SquadSeries, nullptr, true);
			}
			else {
				(*g_GameProgressionManager)->SetAICarCount((*g_GameProgressionManager)->GetAICarCount());
				_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::MissionSettings, nullptr, true);
			}
			break;
		case CarsFrontEndScreen::MainMenu_CustomMissions_SquadSeries:
			_CarsFrontEnd_SetGameModeIndex(_this, _selected_menu);
			_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::MainMenu_MissionSelect_SquadSeries, nullptr, true);
			break;

		case CarsFrontEndScreen::MissionSettings_SquadSeries:
			{
				GameProgressionManager_SetMissionTimeByMode(*g_GameProgressionManager, false);
				GameProgressionManager_FUN_004ebaf0(*g_GameProgressionManager, *(reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(_this) + 0x46C)));
				std::uintptr_t local_34 = *(reinterpret_cast<std::uintptr_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0x3FC));
				while (local_34 != 0 && *reinterpret_cast<CarsFrontEndScreen*>(local_34 + 8) != CarsFrontEndScreen::CustomSquadSeries) {
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
				_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::CustomSquadSeries, "FE_MT_SquadSeries", false);
			}
			break;

		case CarsFrontEndScreen::CustomSquadSeries:
			{
				int index = std::atoi(_selected_menu);
				*(reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(_this) + 0x46C)) = index;
				GameProgressionManager_FUN_004ebab0(*g_GameProgressionManager, index);
				_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::MainMenu_CustomMissions_SquadSeries, nullptr, true);
			}
			break;

		case CarsFrontEndScreen::MissionSettings:
			should_set_screen = true;
			GameProgressionManager_SetMissionTimeByMode(*g_GameProgressionManager, false);
			break;

		case CarsFrontEndScreen::GarageConnect:
			if (_stricmp(_selected_menu, "DLC_Connect") == 0) {
				*(reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(_this) + 0x800)) = 1;
			}
			should_play_item_selected = false;
			break;

		case CarsFrontEndScreen::WorldOfCarsConnect:
			break;

		case CarsFrontEndScreen::MainMenu_Garage:
			if (selected_menu.starts_with("CAR_")) {
				void* dest = (reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(_this) + 0x114));
				Genie_String_Assign(dest, _selected_menu);
				_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::GarageDetails, nullptr, true);
			}
			should_play_item_selected = false;
			*(reinterpret_cast<float*>(reinterpret_cast<std::uintptr_t>(_this) + 0x7A4)) = 16.0f; // FIXME
			*(reinterpret_cast<float*>(reinterpret_cast<std::uintptr_t>(_this) + 0x7A8)) = 8.0f; // FIXME
			break;

		default:
#ifdef MP_STRATEGY_AXEL
			axel::ui::on_confirm(_this, selected_menu);
#endif
			break;
		}
		if (should_set_screen) {
			int locked_controllers = 0;
			for (std::size_t i = 0; i < 11; i++) {
				if ((*g_InputPtr)->IsControllerLocked(i)) {
					locked_controllers += 1;
				}
			}
			PersistentData_SetGlobal(*g_PersistentData, "NumPlayers", locked_controllers);
			PersistentData_SetGlobal(*g_PersistentData, "MultiPlayer", locked_controllers > 1);
			PersistentData_SetGlobal(*g_PersistentData, "ScreenFormat", locked_controllers > 4 ? 1 : locked_controllers);
 			_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::CarSelect, nullptr, true);
		}
		std::uintptr_t movie = *(reinterpret_cast<std::uintptr_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0xbc));
		if (movie != 0 && should_play_item_selected) {
			Flash_Movie_CallFlashFunction(movie, "ItemSelected", nullptr);
			*(reinterpret_cast<std::uint8_t*>(reinterpret_cast<std::uintptr_t>(_this) + 0xC0)) = 1;
		}
		return;

	}
};

DefineReplacementHook(CallFlashVariableFuncHook) {
	static void __fastcall callback(void* _this, uintptr_t edx, struct Movie* movie, char* method, void* args) {
		logger::log_format("[GameSpecificFlashImpl::CallFlashVariableFunc] {}", method);
		original(_this, edx, movie, method, args);
	}
};

static bool IN_APPLY_SETTINGS = false;

DefineReplacementHook(ExternalInterfaceHandler_Callback) {
	static void __fastcall callback(void* _this, uintptr_t edx, struct Movie* movie, char* method, void* args, unsigned int arg_count) {
#ifdef _DEBUG
		logger::log_format("[Flash::Movie::ExternalInterfaceHandler::Callback] {}", method);
#endif
		if (method != nullptr) {
			if (IN_APPLY_SETTINGS && _stricmp(method, "GetMenuOptionsList") == 0) {
				HandleGetMenuOptionsList(movie);
				return;
			}
			if (IN_APPLY_SETTINGS && _stricmp(method, "SelectOption") == 0) {
				HandleSelectOption(movie);
				IN_APPLY_SETTINGS = false;
				return;
			}
			if (_stricmp(method, "GetAllResolutions") == 0) {
				HandleGetAllResolutions(movie);
				return;
			}
			else if (_stricmp(method, "GetCurrResolution") == 0) {
				HandleGetCurrResolution(movie);
				return;
			}
			else if (_stricmp(method, "GetCurrGraphicType") == 0) {
				HandleGetCurrGraphicType(movie);
				return;
			}
			else if (_stricmp(method, "SetCurrGraphicType") == 0) {
				HandleSetCurrGraphicType(movie, std::bit_cast<float>(reinterpret_cast<unsigned int*>(args)[2]));
				return;
			}
			else if (_stricmp(method, "GetCurrLayoutType") == 0) {
				HandleGetCurrLayoutType(movie);
				return;
			}
			else if (_stricmp(method, "SetCurrLayoutType") == 0) {
				HandleSetCurrLayoutType(movie, std::bit_cast<float>(reinterpret_cast<unsigned int*>(args)[2]));
				return;
			}
			else if (_stricmp(method, "ArcadeEssentials_GetMotionBlurEnabled") == 0) {
				HandleGetMotionBlurEnabled(movie);
				return;
			}
			else if (_stricmp(method, "ArcadeEssentials_SetMotionBlurEnabled") == 0) {
				HandleSetMotionBlurEnabled(movie, std::bit_cast<float>(reinterpret_cast<unsigned int*>(args)[2]));
				return;
			}
			else if (_stricmp(method, "PopupMessage") == 0) {
				HandlePopupMessage(movie);
				IN_APPLY_SETTINGS = true;
				return;
			}
			else if (_stricmp(method, "SetCurrResolution") == 0) {
				HandleSetCurrResolution(movie, std::bit_cast<float>(reinterpret_cast<unsigned int*>(args)[2]));
				return;
			}
		}
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

void __fastcall intercept_set_auto_man_drift(void* data, std::uintptr_t edx, const char* name, std::uint32_t default_value) {
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
			_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::SaveSlots, nullptr, false);
		}
		else {
			_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::MT_FrontEnd, nullptr, false);
		}
	}
};

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

DefineInlineHook(InitHudElementsRace) {
	static void _cdecl callback(sunset::InlineCtx& ctx) {
		GameProgressionManager::MissionMode mode = *reinterpret_cast<GameProgressionManager::MissionMode*>(ctx.ebp.unsigned_integer - 0x2c);
		int* local_8 = reinterpret_cast<int*>(ctx.ebp.unsigned_integer - 0x4);

		int playerCount = get_locked_player_count();
#ifdef MP_STRATEGY_AXEL
		if (axel::online()) {
			playerCount = axel::player_count();
		}
#endif

		// assert(mode == GameProgressionManager::MissionMode::Race);

		// 1 Player -> 2, 2 Player -> 3, Else -> 4.
		if (playerCount == 1) {
			*local_8 = 0;
			*reinterpret_cast<int*>(ctx.esp.unsigned_integer) = 2;
		}
		else if (playerCount == 2) {
			*local_8 = 0;
			*reinterpret_cast<int*>(ctx.esp.unsigned_integer) = 3;
		}
		else {
			*local_8 = 0;
			*reinterpret_cast<int*>(ctx.esp.unsigned_integer) = 4;
		}
	}
};

DefineInlineHook(InitHudElements) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		GameProgressionManager::MissionMode mode = *reinterpret_cast<GameProgressionManager::MissionMode*>(ctx.ebp.unsigned_integer - 0x2c);
		int* local_8 = reinterpret_cast<int*>(ctx.ebp.unsigned_integer - 0x4);

		int playerCount = get_locked_player_count();
#ifdef MP_STRATEGY_AXEL
		if (axel::online()) {
			playerCount = axel::player_count();
		}
#endif

		if (mode == GameProgressionManager::MissionMode::Bomb || mode == GameProgressionManager::MissionMode::Hunter || mode == GameProgressionManager::MissionMode::Arena) {
			*local_8 = 7;
			// 1 Player -> 15, 2 Player -> 16, Else -> 17
			if (playerCount == 1) {
				*reinterpret_cast<int*>(ctx.esp.unsigned_integer) = 15;
			}
			else if (playerCount == 2) {
				*reinterpret_cast<int*>(ctx.esp.unsigned_integer) = 16;
			}
			else {
				*reinterpret_cast<int*>(ctx.esp.unsigned_integer) = 17;
			}
		}
		else if (mode == GameProgressionManager::MissionMode::Collect) {
			*local_8 = 4;
			// 1 Player -> 10, 2 Player -> 11, Else -> 12
			if (playerCount == 1) {
				*reinterpret_cast<int*>(ctx.esp.unsigned_integer) = 10;
			}
			else if (playerCount == 2) {
				*reinterpret_cast<int*>(ctx.esp.unsigned_integer) = 11;
			}
			else {
				*reinterpret_cast<int*>(ctx.esp.unsigned_integer) = 12;
			}
		}
		else if (mode == GameProgressionManager::MissionMode::Takedown) {
			*local_8 = 3;
			// 1 Player -> 5, Else -> 6
			if (playerCount == 1) {
				*reinterpret_cast<int*>(ctx.esp.unsigned_integer) = 5;
			}
			else {
				*reinterpret_cast<int*>(ctx.esp.unsigned_integer) = 6;
			}
		}
		else if (mode == GameProgressionManager::MissionMode::Tutorial) {
			*local_8 = 1;
			// 1 Player -> 7, Else -> 8
			if (playerCount == 1) {
				*reinterpret_cast<int*>(ctx.esp.unsigned_integer) = 7;
			}
			else {
				*reinterpret_cast<int*>(ctx.esp.unsigned_integer) = 8;
			}
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
		std::uintptr_t _this = *reinterpret_cast<std::uintptr_t*>(ctx.ebp.unsigned_integer - 0xCC);
		int playerNumber = *reinterpret_cast<int*>(_this + 0x194);

		std::uintptr_t* inst = *reinterpret_cast<std::uintptr_t**>(ctx.ebp.unsigned_integer - 0x18);
		Genie::String* buttonTurbo = reinterpret_cast<Genie::String*>(ctx.ebp.unsigned_integer - 0x3C);
		buttonTurbo->Append(SuffixForCT((*g_InputPtr)->GetController(playerNumber)->m_uiType).data());

		auto func = *reinterpret_cast<void(__thiscall**)(void*, char*, const char*, const char*)>(*inst + 0x50);
		func(inst, buttonTurbo->data, "turbo", "normal");
	}
};

DefineInlineHook(HandleHudPositionMulti) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		if ((ctx.eax.unsigned_integer & 0x10000) != 0) {
			void* _inst = operator_new(0x74);
			HudPosition_Multi_HudPosition_Multi(_inst, ctx.edx.unsigned_integer + 0x5C, *reinterpret_cast<std::uint32_t*>(ctx.edx.unsigned_integer + 0x9c));
			*reinterpret_cast<void**>(ctx.edx.unsigned_integer + 0x34) = _inst;
		}
	}
};

// https://stackoverflow.com/questions/36543301/detecting-windows-10-version
RTL_OSVERSIONINFOW windows_version() {
	HMODULE ntdll = ::GetModuleHandleW(L"ntdll.dll");
	if (ntdll != nullptr) {
		auto rtl_get_version = reinterpret_cast<NTSTATUS(WINAPI*)(PRTL_OSVERSIONINFOW)>(GetProcAddress(ntdll, "RtlGetVersion"));
		if (rtl_get_version != nullptr) {
			RTL_OSVERSIONINFOW info = { 0 };
			info.dwOSVersionInfoSize = sizeof(info);
			if (rtl_get_version(&info) == 0) {
				return info;
			}
		}
	}
	return RTL_OSVERSIONINFOW{};
}

// https://github.com/hedge-dev/UnleashedRecomp/blob/main/UnleashedRecomp/ui/game_window.cpp
DefineInlineHook(DarkModeWindowTitle) {
	static void _cdecl callback(sunset::InlineCtx& ctx) {
		RTL_OSVERSIONINFOW version = windows_version();
		if (version.dwMajorVersion >= 10 && version.dwBuildNumber >= 17763) {
			const std::uint32_t use_immersive_dark_mode = 1;
			// DWMWA_USE_IMMERSIVE_DARK_MODE
			DwmSetWindowAttribute(*reinterpret_cast<HWND*>(ctx.edx.unsigned_integer + 0x4), version.dwBuildNumber >= 18985 ? 20 : 19, &use_immersive_dark_mode, sizeof(use_immersive_dark_mode));
		}
	}
};

DefineInlineHook(CheckForExitToWindows) {
	static void _cdecl callback(sunset::InlineCtx& ctx) {
		const char* _str = *reinterpret_cast<const char**>(ctx.ebp.unsigned_integer - 0x4);
		if (_stricmp(_str, "Win32Wii_Scn_ExitToWindows") == 0) {
			FUN_004d0250(*reinterpret_cast<std::uintptr_t*>(*reinterpret_cast<std::uintptr_t*>(ctx.ebp.unsigned_integer - 0xC8) + 4), 2);
		}
	}
};

DefineInlineHook(CheckForExitToWindows2) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		int selected_option = *reinterpret_cast<int*>(*reinterpret_cast<std::uintptr_t*>(ctx.edx.unsigned_integer + 4) + 0x18);
		if (selected_option == 2) {
			std::exit(0);
		}
	}
};

DefineInlineHook(CheckForGraphicsMenu) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		std::uintptr_t _this = *reinterpret_cast<std::uintptr_t*>(ctx.ebp.unsigned_integer - 0x2F4);
		const char* _selected_menu = *reinterpret_cast<const char**>(_this + 0x40C);
		if (_selected_menu != nullptr) {
			if (_stricmp(_selected_menu, "FE_EX_Graphics") == 0) {
				*reinterpret_cast<const char**>(ctx.esp.unsigned_integer) = "Win32Wii_Scn_Item_Resolution,Win32Wii_Scn_GraphicQuality,Win32Wii_Scn_ScreenLayout,ArcadeEssentials_MotionBlur";
			}
		}
	}
};

DefineInlineHook(CheckForGraphicsMenu2) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		const char* _selected_menu = *reinterpret_cast<const char**>(ctx.eax.unsigned_integer + 0x40C);
		if (_selected_menu != nullptr) {
			if (_stricmp(_selected_menu, "FE_EX_Graphics") == 0) {
				*reinterpret_cast<const char**>(ctx.esp.unsigned_integer) = "text,text,text,text";
			}
		}
	}
};

DefineInlineHook(CheckForGraphicsMenu3) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		std::uintptr_t _this = *reinterpret_cast<std::uintptr_t*>(ctx.ebp.unsigned_integer + -0x184);
		const char* _selected_menu = *reinterpret_cast<const char**>(_this + 0x40C);
		if (_selected_menu != nullptr) {
			if (_stricmp(_selected_menu, "FE_EX_Graphics") == 0) {
				*reinterpret_cast<const char**>(ctx.ebp.unsigned_integer + -0x14) = "pc_graphic_setting";
			}
		}
	}
};

DefineInlineHook(GetTypeCCI) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		char* screen_type = *reinterpret_cast<char**>(ctx.ebp.unsigned_integer + 0xC);
		if (_stricmp(screen_type, "FS") == 0) {
			*reinterpret_cast<int*>(ctx.ebp.unsigned_integer - 0x10) = 0;
		}
		else if (_stricmp(screen_type, "HH") == 0) {
			*reinterpret_cast<int*>(ctx.ebp.unsigned_integer - 0x10) = 1;
		}
		else if (_stricmp(screen_type, "HHHW") == 0) {
			*reinterpret_cast<int*>(ctx.ebp.unsigned_integer - 0x10) = 2;
		}
		else {
			*reinterpret_cast<int*>(ctx.ebp.unsigned_integer - 0x10) = 0;
		}
	}
};

DefineInlineHook(FixDefaultDistance) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		*reinterpret_cast<std::uint32_t*>(ctx.eax.unsigned_integer + 0x260) = *reinterpret_cast<std::uint32_t*>(ctx.ebp.unsigned_integer - 0x24);
	}
};

DefineReplacementHook(FlashGuiImpRender) {
	static void _fastcall callback(std::uintptr_t _this, std::uintptr_t edx, void* pMovieView, void* pTexture, void* movieInfo, float x, float y, float w, float h) {
		auto* device = *reinterpret_cast<IDirect3DDevice9Ex**>(0x01906334);
		// Backup the SrgbTexture value for sampler 0.
		unsigned long srgb_sampler_state_backup = 0;
		device->GetSamplerState(0, D3DSAMP_SRGBTEXTURE, &srgb_sampler_state_backup);
		// Force SrgbTexture for sampler 0 to FALSE before rendering the UI.
		device->SetSamplerState(0, D3DSAMP_SRGBTEXTURE, 0);
		original(_this, edx, pMovieView, pTexture, movieInfo, x, y, w, h);
		// Restore sampler 0 back to its original state.
		device->SetSamplerState(0, D3DSAMP_SRGBTEXTURE, srgb_sampler_state_backup);
	}
};

struct GViewport {
	int bufferWidth;
	int bufferHeight;
	int left;
	int top;
	int width;
	int height;
	int scissorLeft;
	int scissorTop;
	int scissorWidth;
	int scissorHeight;
	float scale;
	float aspectRatio;
	int flags;
};

static_assert(sizeof(GViewport) == 0x34);

DefineInlineHook(AdjustScaleformViewport) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		GViewport* view = reinterpret_cast<GViewport*>(ctx.ecx.unsigned_integer);

		float actualW = static_cast<float>(Resolution::current().width);
		float actualH = static_cast<float>(Resolution::current().height);
		float aspect = actualW / actualH;

		const float targetAspect = 1280.0f / 720.0f;
		float scaleFactor = 1.0f;

		if (aspect >= targetAspect) {
			// bluedragon: 16:9 and wider...
			scaleFactor = 720.0f / actualH;
		}
		else {
			// bluedragon: Anything narrower than 16:9...
			scaleFactor = 1280.0f / actualW;
		}

		// bluedragon: Remove cold ball symptoms from UI (No shrinking at lower resolutions due to larger canvas shrinking UI elements)
		if (scaleFactor > 1.0f) {
			scaleFactor = 1.0f;
		}

		view->scale = scaleFactor;
	}
};

DefineReplacementHook(MiniMenuAddButton) {
	static void __fastcall callback(std::uintptr_t* _this, std::uintptr_t edx, const char* icon_key, const char* label_key) {
		if (*_this != 0) {
			std::string key = icon_key;
			key += SuffixForCT((*g_InputPtr)->GetController(0)->m_uiType);
#ifdef _DEBUG
			logger::log_format("[MiniMenu::AddButton] Added button: {}, {}.", key.data(), label_key);
#endif
			Flash_Movie_CallFlashFunction(*_this, "AddButton", 0, key.data(), label_key);
		}
	}
};

DefineReplacementHook(MiniMenuShowTite) {
	static void __fastcall callback(std::uintptr_t * _this) {
		if (*_this != 0) {
			Flash_Movie_CallFlashFunction(*_this, "ShowTitle", 0);
		}
	}
};

DefineReplacementHook(MiniMenuResetButtons) {
	static void __fastcall callback(std::uintptr_t * _this) {
		if (*_this != 0) {
#ifdef _DEBUG
			logger::log_format("[MiniMenu::ResetButtons] Removed all buttons.");
			logger::log_format("[MiniMenu::ResetButtons] Added `menu_button_main`, `Scn_Select`.");
			logger::log_format("[MiniMenu::ResetButtons] Added `menu_button_main-cancel`, `SharedText_Back`.");
#endif	
			std::string main = "menu_button_main";
			std::string back_cancel = "menu_button_back-cancel";
			main += SuffixForCT((*g_InputPtr)->GetController(0)->m_uiType);
			back_cancel += SuffixForCT((*g_InputPtr)->GetController(0)->m_uiType);

			Flash_Movie_CallFlashFunction(*_this, "RemoveAllButtons", 0);
			Flash_Movie_CallFlashFunction(*_this, "AddButton", 0, main.data(), "Scn_Select");
			Flash_Movie_CallFlashFunction(*_this, "AddButton", 0, back_cancel.data(), "SharedText_Back");
		}
	}
};

DefineReplacementHook(MiniMenuSwapFirstButton) {
	static void __fastcall callback(std::uintptr_t * _this, std::uintptr_t edx, char* icon_key, const char* label_key) {
		if (*_this != 0) {
			std::string key = icon_key;
			key += SuffixForCT((*g_InputPtr)->GetController(0)->m_uiType);
			Flash_Movie_CallFlashFunction(*_this, "SwapFirstButton", 0, key.data(), label_key);
		}
	}
};

DefineInlineHook(AddButtons) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		std::uintptr_t movie = **reinterpret_cast<std::uintptr_t**>(ctx.ebp.unsigned_integer - 0xc);

		std::string main = "menu_button_main";
		std::string back_cancel = "menu_button_back-cancel";
		main += SuffixForCT((*g_InputPtr)->GetController(0)->m_uiType);
		back_cancel += SuffixForCT((*g_InputPtr)->GetController(0)->m_uiType);

		Flash_Movie_CallFlashFunction(movie, "AddButton", 0, main.data(), "Scn_Select");
		Flash_Movie_CallFlashFunction(movie, "AddButton", 0, back_cancel.data(), "SharedText_Back");
	}
};

DefineInlineHook(SetPIPWidth) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		double scale = static_cast<double>(Resolution::current().height) / 720.0;
		ctx.eax.unsigned_integer *= scale;
	}
};

DefineInlineHook(SetPIPHeight) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		double scale = static_cast<double>(Resolution::current().height) / 720.0;
		ctx.eax.unsigned_integer *= scale;
	}
};

DefineInlineHook(SetShadowResolution) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		*reinterpret_cast<unsigned int*>(ctx.ebp.unsigned_integer - 0x08) = resolve_shadow_quality(GLOBAL_CONFIG->shadow_quality);
		*reinterpret_cast<unsigned int*>(ctx.ebp.unsigned_integer - 0x1C) = resolve_shadow_quality(GLOBAL_CONFIG->shadow_quality);
		// Set the cascade count to 3 if we're using the `shadowmask_c3_rot8_nodb_ss_cs` shader.
		if (is_shadow_quality_high(GLOBAL_CONFIG->shadow_quality)) {
			*reinterpret_cast<unsigned int*>(ctx.ebp.unsigned_integer - 0x10) = 3;
		}
	}
};

DefineInlineHook(SetEnableHiResShadows) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		ctx.ecx.unsigned_integer = is_shadow_quality_high(GLOBAL_CONFIG->shadow_quality) ? 1 : 0;
	}
};

DefineInlineHook(SetEnableHiResShadows2) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		ctx.eax.unsigned_integer = is_shadow_quality_high(GLOBAL_CONFIG->shadow_quality) ? 1 : 0;
	}
};

DefineInlineHook(SetMotionBlurHook) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		ctx.ecx.unsigned_integer = GLOBAL_CONFIG->motion_blur ? 0 : 1;
	}
};

struct VertexStreamLookup {
	D3DDECLTYPE d3d;
	Renderer::DataType octane;
};

const std::array<VertexStreamLookup, 17> VERTEX_STREAM_CONVERSION_TABLE{
	VertexStreamLookup{D3DDECLTYPE_FLOAT1,    Renderer::DataType::FLOAT_1},
	VertexStreamLookup{D3DDECLTYPE_FLOAT2,    Renderer::DataType::FLOAT_2},
	VertexStreamLookup{D3DDECLTYPE_FLOAT3,    Renderer::DataType::FLOAT_3},
	VertexStreamLookup{D3DDECLTYPE_FLOAT4,    Renderer::DataType::FLOAT_4},
	VertexStreamLookup{D3DDECLTYPE_UBYTE4,    Renderer::DataType::U8_4},
	VertexStreamLookup{D3DDECLTYPE_SHORT2,    Renderer::DataType::S16_2}, // Added
	VertexStreamLookup{D3DDECLTYPE_SHORT4,    Renderer::DataType::S16_4}, // Added
	VertexStreamLookup{D3DDECLTYPE_UBYTE4N,   Renderer::DataType::U8_4_NORMALIZED},
	VertexStreamLookup{D3DDECLTYPE_SHORT2N,   Renderer::DataType::S16_2_NORMALIZED},
	VertexStreamLookup{D3DDECLTYPE_SHORT4N,   Renderer::DataType::S16_4_NORMALIZED},
	VertexStreamLookup{D3DDECLTYPE_USHORT2N,  Renderer::DataType::U16_2_NORMALIZED}, // Added
	VertexStreamLookup{D3DDECLTYPE_USHORT4N,  Renderer::DataType::U16_4_NORMALIZED}, // Added
	VertexStreamLookup{D3DDECLTYPE_UDEC3,     Renderer::DataType::U10_10_10}, // Added
	VertexStreamLookup{D3DDECLTYPE_DEC3N,     Renderer::DataType::S10_10_10_NORMALIZED}, // Added
	VertexStreamLookup{D3DDECLTYPE_FLOAT16_2, Renderer::DataType::FLOAT16_2},
	VertexStreamLookup{D3DDECLTYPE_FLOAT16_4, Renderer::DataType::FLOAT16_4},
	VertexStreamLookup{D3DDECLTYPE_UNUSED,    Renderer::DataType::INVALID_TYPE},
};

DefineReplacementHook(RendererConvertTo) {
	static D3DDECLTYPE _cdecl callback(Renderer::DataType in) {
		for (const auto& elem : VERTEX_STREAM_CONVERSION_TABLE) {
			if (elem.octane == in) {
				return elem.d3d;
			}
		}
		return D3DDECLTYPE_UNUSED;
	}
};

DefineReplacementHook(LockPlayerToController) {
	static void _fastcall callback(WindowsSystemInputDriver * _this, std::uintptr_t edx, int player, int controller) {
		logger::log_format("[WindowsSystemInputDriver::LockPlayerToController] Locked Player: {} -> Controller {}", player, controller);
		original(_this, edx, player, controller);
	}
};

DefineReplacementHook(FixChallengeManagerUnk) {
	static bool __fastcall callback(void* _this, std::uintptr_t edx, int param_2, unsigned int param_3, unsigned int actorHandle) {
		if (actorHandle == 0) {
			if (FUN_00f677b0(*reinterpret_cast<void**>(0x0192c5ec), param_2) == 0) {
				FUN_00f66ca0(*reinterpret_cast<void**>(0x0192c5ec), param_2, param_3);
				return true;
			}
		}
		else {
			CActor* actor = CActor::FromHandle(actorHandle);
			if ((!(*g_RaceManager)->RacerIsAIControlled(actorHandle, -1)) && actor != nullptr) {
				Avatar* avatar = Avatar::FromCActor(actor);
				if (avatar == nullptr) {
					if (FUN_00f677b0(*reinterpret_cast<void**>(0x0192c5ec), param_2) == 0) {
						FUN_00f66ca0(*reinterpret_cast<void**>(0x0192c5ec), param_2, param_3);
						return true;
					}
				}
				else {
					int iVar2 = FUN_00f677b0(*reinterpret_cast<void**>(0x0192c5ec), param_2);
					if (iVar2 == 0) {
						FUN_00f66ca0(*reinterpret_cast<void**>(0x0192c5ec), param_2, param_3);
						return true;
					}
					else if (iVar2 == 1) {
						FUN_00f66cf0(*reinterpret_cast<void**>(0x0192c5ec), param_2, avatar->GetPlayerId(), param_3);
						return true;
					}
				}
			}
		}
		return false;
	}
};

DefineReplacementHook(CarsGame_PlayIntroVideos) {
	static void __fastcall callback(void* _this) {
		AudioEventManager_SetVolume(*g_AudioEventManager, 1, 1.0, 0.0);
		(*g_FlashGui)->Initialize(1, 0x400000);
		Flash::Movie* subtitlesMovie = (*g_FlashGui)->AddMovie("subtitles", true, -1.0, -1.0);
		Subtitles subtitles(subtitlesMovie, false);
		(*g_FlashGui)->Update();

		FMV::Player player{};
		player.Begin();
		bool aborted = false;
		float delayTime = 70.0f;
		
		logger::log_format("[CarsGame::PlayIntroVideos] Video `delayTime`: {}", delayTime);
		
		player.Queue("%1/Pixar_Logo", true, delayTime, false, false);
		player.Play(&aborted);
		player.Queue("%1/avlogo", true, 0.0, false, false);
		player.Play(&aborted);
		player.Queue("%1/gslogo", true, 0.0, false, false);
		player.Play(&aborted);
		player.Queue("%1/DIS", true, delayTime, false, false);
		player.Play(&aborted);
		player.Queue("%1/Copyright", false, 0.0, false, false);
		player.Play(&aborted);
		player.Queue("%1/Opening", true, delayTime, false, false);
		player.Play(&aborted);
		player.End();

		if (subtitlesMovie != nullptr) {
			(*g_FlashGui)->RemoveMovie(&subtitlesMovie, false);
		}

		(*g_FlashGui)->Terminate(1, false);
	}
};

DefineReplacementHook(HudSkateReticuleHook) {
	static void* __fastcall callback(void* _this, std::uintptr_t edx, unsigned int param_2, float param_3, float param_4) {
		float actualW = static_cast<float>(Resolution::current().width);
		float actualH = static_cast<float>(Resolution::current().height);
		float aspect = actualW / actualH;

		constexpr double targetAspect = 1280.0 / 720.0;
		float scaleFactor = 1.0f;

		if (aspect >= targetAspect) {
			scaleFactor = 720.0f / actualH;
		}
		else {
			scaleFactor = 1280.0f / actualW;
		}

		// bluedragon: Reticle alightment low res fix
		if (scaleFactor > 1.0f) {
			scaleFactor = 1.0f;
		}

		return original(_this, edx, param_2, param_3 * scaleFactor, param_4 * scaleFactor);
	}
};

DefineInlineHook(BackwardSteeringInvert) {
	static void __cdecl callback(sunset::InlineCtx& ctx) {
		CarsVehicle* vehicle = reinterpret_cast<CarsVehicle*>(ctx.ecx.unsigned_integer);
		if (vehicle->IsAIControlled()) {
			return;
		}
		if (vehicle->GetActiveMoves()->m_actionState != ActiveMoves::ActionState::BackwardsDriving) {
			return;
		}
		float* steeringInput = reinterpret_cast<float*>(reinterpret_cast<std::uintptr_t>(vehicle) + 0x8e8);
		*steeringInput = -*steeringInput;
	}
};

DefineInlineHook(FixBwdEyeL) {
	static void __cdecl callback(sunset::InlineCtx& ctx) {
		CarsVehicle* vehicle = *reinterpret_cast<CarsVehicle**>(ctx.ebp.unsigned_integer - 0x400);
		if (vehicle->IsAIControlled()) {
			return;
		}
		*reinterpret_cast<const char**>(ctx.esp.unsigned_integer) = "emo_bwd_R";
	}
};

DefineInlineHook(FixBwdEyeR) {
	static void __cdecl callback(sunset::InlineCtx& ctx) {
		CarsVehicle* vehicle = *reinterpret_cast<CarsVehicle**>(ctx.ebp.unsigned_integer - 0x400);
		if (vehicle->IsAIControlled()) {
			return;
		}
		*reinterpret_cast<const char**>(ctx.esp.unsigned_integer) = "emo_bwd_L";
	}
};

DefineInlineHook(FixBwdLean) {
	static void __cdecl callback(sunset::InlineCtx& ctx) {
		CarsVehicle* vehicle = *reinterpret_cast<CarsVehicle**>(ctx.ebp.unsigned_integer - 0x400);
		if (vehicle->IsAIControlled()) {
			return;
		}
		if (vehicle->GetActiveMoves()->m_actionState != ActiveMoves::ActionState::BackwardsDriving) {
			return;
		}

		float* leanVal = reinterpret_cast<float*>(ctx.ebp.unsigned_integer - 0x188);
		// bluedragon: IT WAS A FRIGGIN BLEND WEIGHT
		*leanVal = 1.0f - *leanVal;
	}
};

DefineReplacementHook(DisplayWeaponTutorialHook) {
	static void __fastcall callback(CarsWeaponInventory* _this) {
		int playerNum = (*g_RaceManager)->GetPlayerNumber(*CActor::FromHandle(_this->GetActor().GetHandle()));
		HudActiveInfo* activeInfo = (*g_CarsHud)->hud[playerNum]->m_hudActiveInfo;
		if (activeInfo != nullptr) {
			if ((*g_InputPtr)->GetController(playerNum) != nullptr) {
				std::string fireButton = "button_fire";
				fireButton += SuffixForCT((*g_InputPtr)->GetController(playerNum)->m_uiType);
				activeInfo->SetTextAndPlayAnim(CTranslator_Translate(reinterpret_cast<void*>(0x0192674c), "WeaponUse", true), WEAPON_TUTORIAL_LIVE_TIME, CTranslator_Translate(reinterpret_cast<void*>(0x0192674c), fireButton.data(), true), "normal", false);
			}
		}
	}
};

DefineInlineHook(FixFireButtonUI) {
	static void __cdecl callback(sunset::InlineCtx& ctx) {
		int index = *reinterpret_cast<int*>(ctx.ebp.unsigned_integer - 0x18);
		Genie::String* buttonFire = reinterpret_cast<Genie::String*>(ctx.ebp.unsigned_integer - 0x1C);
		buttonFire->Append(SuffixForCT((*g_InputPtr)->GetController(index)->m_uiType).data());
	}
};

DefineInlineHook(FixRevItButtonUI) {
	static void __cdecl callback(sunset::InlineCtx & ctx) {
		Genie::String* buttonRevIt = reinterpret_cast<Genie::String*>(ctx.ebp.unsigned_integer - 0xEC);
		HudGauge* _this = *reinterpret_cast<HudGauge**>(ctx.ebp.unsigned_integer - 0x118);
		int playerId = 0;
		for (auto i = 0; i < 4; i++) {
			if ((*g_CarsHud)->hud[i] != nullptr) {
				if ((*g_CarsHud)->hud[i]->m_hudGuage == _this) {
					playerId = i;
				}
			}
		}
		buttonRevIt->Append(SuffixForCT((*g_InputPtr)->GetController(playerId)->m_uiType).data());
	}
};

DefineInlineHook(FixHudTutorialButtonPrompt) {
	static void __cdecl callback(sunset::InlineCtx & ctx) {
		std::uintptr_t handler = *reinterpret_cast<std::uintptr_t*>(ctx.ebp.unsigned_integer - 0x40);
		std::uintptr_t userData = *reinterpret_cast<std::uintptr_t*>(handler + 0x14);
		int playerNum = *reinterpret_cast<int*>(userData + 0x80);
		std::string prompt = *reinterpret_cast<char**>(ctx.ebp.unsigned_integer - 0x10);
		prompt += SuffixForCT((*g_InputPtr)->GetController(playerNum)->m_uiType).data();
		char* translated = CTranslator_Translate(reinterpret_cast<void*>(0x0192674c), prompt.data(), true);
		ctx.eax.unsigned_integer = reinterpret_cast<std::uintptr_t>(translated);
	}
};

DefineInlineHook(FixAutoDriftButtonUI) {
	static void __cdecl callback(sunset::InlineCtx & ctx) {
		int index = *reinterpret_cast<int*>(ctx.ebp.unsigned_integer - 0x24);
		Genie::String* feAutoDriftButton = reinterpret_cast<Genie::String*>(ctx.ebp.unsigned_integer - 0x28);
		feAutoDriftButton->Append(SuffixForCT((*g_InputPtr)->GetController(index)->m_uiType).data());
	}
};

DefineInlineHook(FixLoadTipUI) {
	static void __cdecl callback(sunset::InlineCtx & ctx) {
		ControllerInputDriver* controller = (*g_InputPtr)->GetAssertController();
		if ((*g_InputPtr)->ControllerLocked(0)) {
			controller = (*g_InputPtr)->GetController(0);
		}
		bool isControllerSpecific = *reinterpret_cast<bool*>(ctx.ebp.unsigned_integer - 0x111);
		if (controller != nullptr && isControllerSpecific) {
			ctx.eax.unsigned_integer = reinterpret_cast<std::uintptr_t>(LoadTipSuffixForCT(controller->m_uiType).data());
		}
	}
};

DefineReplacementHook(FlashControlMapperUpdateHook) {
	static void __fastcall callback(Flash::FlashControlMapper * _this) {
		ControlMapper_Update(_this);
		if (0.0 < _this->timer) {
			_this->timer = _this->timer - static_cast<float>(*g_OSFrameMS) * 0.001;
		}
		if (_this->applyDelay != false) {
			_this->buttonDownDelay = CMasterTimer_GetOSTime() + _this->buttonWaitTime;
			_this->applyDelay = false;
		}
		if (_this->buttonDownDelay != 0) {
			if (_this->buttonDownDelay < CMasterTimer_GetOSTime()) {
				_this->buttonDownDelay = 0;
			}
		}

		if (_this->device == nullptr) {
			return;
		}
		if (!_this->device->Connected()) {
			return;
		}

		_this->SetActionMapPlatform(ActionMapPlatformForCT(_this->device->m_uiType).data());
	}
};

DefineInlineHook(FlashInputActionMapHook) {
	static void __cdecl callback(sunset::InlineCtx & ctx) {
		ControllerInputDriver* controller = (*g_InputPtr)->GetUnlockedController(0);
		if (controller != nullptr) {
			*reinterpret_cast<const char**>(ctx.esp.unsigned_integer) = (ActionMapPlatformForCT(controller->m_uiType).data());
		}
	}
};

DefineReplacementHook(CCMSetPlayerID) {
	static void __fastcall callback(CarsControlMapper * _this, std::uintptr_t, int playerId) {
		_this->m_playerId = playerId;
		_this->m_driver = (*g_InputPtr)->GetController(playerId);
		if (_this->m_driver != nullptr) {
			_this->SwitchControlScheme(XMLNameForCT(_this->m_driver->m_uiType).data());
		}
	}
};

DefineReplacementHook(TransDebugHook) {
	static char* __fastcall callback(void * _this, std::uintptr_t, const char* label, bool unk) {
		char* result = original(_this, 0, label, unk);
		logger::log_format("[CTranslator::Translate] Mapped Devon Key: `{}` -> `{}`", label, result);
		return result;
	}
};

struct TranslateInfo {
	wchar_t* pKey;
	void* pResult;
	char* pInstanceName;
	unsigned char Flags;
};

std::string WideToUTF8(const std::wstring& wstr) {
	std::string retStr;
	if (!wstr.empty()) {
		int sizeRequired = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
		if (sizeRequired > 0) {
			std::vector<char> utf8String(sizeRequired);
			int bytesConverted = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &utf8String[0], utf8String.size(), nullptr, nullptr);
			if (bytesConverted != 0) {
				retStr = &utf8String[0];
			}
			else {
				return "";
			}
		}
	}
	return retStr;
}

std::wstring UTF8ToWide(const std::string& str) {
	std::wstring retStr;
	if (!str.empty()) {
		int sizeRequired = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
		if (sizeRequired > 0) {
			std::vector<wchar_t> wideString(sizeRequired);
			int charsConverted = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wideString[0], wideString.size());
			if (charsConverted != 0) {
				retStr = &wideString[0];
			}
			else {
				return L"";
			}
		}
	}
	return retStr;
}

DefineReplacementHook(FlashToDevonHook) {
	static void __fastcall callback(void* _this, std::uintptr_t, TranslateInfo * info) {
		wchar_t* key = info->pKey;
		if (key[0] == L'@') {
			std::string sanitized = WideToUTF8(info->pKey + 1);
			if (key[1] == L'^') {
				int controllerID = static_cast<int>(key[2]) - '0';
				sanitized = sanitized.substr(2);
				if ((*g_InputPtr)->GetController(controllerID) != nullptr) {
					sanitized += SuffixForCT((*g_InputPtr)->GetController(controllerID)->m_uiType);
				}
			}
			char* localized = CTranslator_Translate(reinterpret_cast<void*>(0x0192674c), sanitized.data(), false);
			if (localized == nullptr) {
				sanitized = "[" + sanitized + "]";
				GFxTranslator_TranslateInfo_SetResult_UTF8(info, sanitized.data(), -1);
			}
			else {
				std::wstring wideLocalized = UTF8ToWide(localized);
				if ((*g_FlashGui)->UsingRightToLeftFont()) {
					FlipEnglishWords(wideLocalized.data(), wideLocalized.size());
				}
				GFxTranslator_TranslateInfo_SetResult_UTF16(info, wideLocalized.data(), -1);
				info->Flags = info->Flags | 2;
			}
		}
		else {
#ifdef _DEBUG
			std::string sanitized = WideToUTF8(info->pKey);
			logger::log_format("[Flash::Movie::FlashToDevon::Translate] Untranslated Devon Key: {}!", sanitized);
#endif
			if ((*g_FlashGui)->UsingRightToLeftFont()) {
				FlipEnglishWords(key, wcslen(key));
			}
			GFxTranslator_TranslateInfo_SetResult_UTF16(info, key, -1);
			info->Flags = info->Flags | 2;
		}
		return;
	}
};

DefineReplacementHook(CarsFrontEnd_ShowControllerPull_HandleMessage) {
	static void __fastcall callback(GenericMessageHandler<CarsFrontEnd>* _this, std::uintptr_t edx, ShowControllerPullPacket* udata, std::uint32_t actor_handle, std::uint32_t posted) {
		if (_this->m_reference->m_isUpdating) {
			bool isMp = false;
			if (_this->m_reference->currentScreen == CarsFrontEndScreen::CarSelect) {
				if (PersistentData_GetGlobal(*g_PersistentData, "MultiPlayer") != 0) {
					isMp = true;
				}
			}
			int index = (*g_InputPtr)->GetPlayerIndex((*g_InputPtr)->GetUnlockedController(udata->playerIndex));
			if (isMp || index == 0) {
				ControllerInputDriver* controller = (*g_InputPtr)->GetUnlockedController(udata->playerIndex);
				if (controller != nullptr) {
					logger::log_format("[ShowControllerPull::HandleMessage] Connected: {}", controller->Connected());
					if (controller->Connected() && udata->type == 2) {
						if (_this->m_reference->field34_0xb8 != nullptr) {
							if (isMp) {
								Flash_Movie_CallFlashFunction(reinterpret_cast<std::uintptr_t>(_this->m_reference->field34_0xb8), "SetAutoDriftButton", 0);
							}
							else if (_this->m_reference->currentScreen == CarsFrontEndScreen::CarSelect) {
								Flash_Movie_CallFlashFunction(reinterpret_cast<std::uintptr_t>(_this->m_reference->field34_0xb8), "UpdateAutoMan", 0);
							}
						}

						if (_this->m_reference->miniMenu != nullptr && index == 0) {
							Flash_Movie_CallFlashFunction(reinterpret_cast<std::uintptr_t>(_this->m_reference->miniMenu->movie), "SwapButtons", 0, SuffixForCT(controller->m_uiType).data());
						}
					}
				}
			}

		}
		
	}
};

DefineReplacementHook(ErrorPopup_HideControllerPullMessage_HandleMessage) {
	static void __fastcall callback(GenericMessageHandler<void>*_this, std::uintptr_t edx, HideControllerPullPacket* udata, std::uint32_t actor_handle, std::uint32_t posted) {
		original(_this, edx, udata, actor_handle, posted);

		Avatar* avatar = reinterpret_cast<Avatar*>(Players_GetAvatarFromPlayerId(udata->playerIndex));
		if (avatar != nullptr) {
			if (avatar->IsPlayerControlled()) {
				CarsControlMapper* ccm = Cars2VehicleDBlock::Get(*avatar)->m_carsControlMapper;
				ccm->m_driver = (*g_InputPtr)->GetController(udata->playerIndex);
				if (ccm->m_driver != nullptr) {
					Flash_Movie_CallFlashFunction(reinterpret_cast<std::uintptr_t>((*g_CarsHud)->hud[udata->playerIndex]->m_movie), "SwapButtons", 0, SuffixForCT(ccm->m_driver->m_uiType).data());
					ccm->SwitchControlScheme(XMLNameForCT(ccm->m_driver->m_uiType).data());
				}
				(*g_RaceManager)->UpdateRacerControlContexts();
			}
		}
	}
};

#ifdef MP_STRATEGY_AXEL

DefineReplacementHook(GetOptionsListHook) {
	static void __fastcall callback(CarsFrontEnd * _this, std::uintptr_t edx, char** outOptionsList) {
		CarsFrontEndScreen screen = *reinterpret_cast<CarsFrontEndScreen*>(reinterpret_cast<std::uintptr_t>(_this) + 0xA8);
		if (auto optionsList = axel::ui::retrieve_options_list(_this, screen); optionsList.has_value()) {
			std::string value = optionsList.value();
			*outOptionsList = reinterpret_cast<char*>(operator_new_array(value.size() + 1));
			std::memcpy(*outOptionsList, value.data(), value.size());
			(*outOptionsList)[value.size()] = 0;
		}
		else {
			original(_this, edx, outOptionsList);
		}
	}
};

DefineReplacementHook(CarsFrontEnd_UpdateHook) {
	static void __fastcall callback(CarsFrontEnd * _this) {
		axel::ui::update(_this);
		original(_this);
	}
};

DefineInlineHook(HandleForeignSetScreen) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		std::uintptr_t _this = *reinterpret_cast<std::uintptr_t*>(ctx.ebp.unsigned_integer - 0x184);
		CarsFrontEndScreen screen = *reinterpret_cast<CarsFrontEndScreen*>(_this + 0xA8);
		logger::log_format("[CarsFrontEnd::SetScreen] Entering Foreign Screen: {}", static_cast<int>(screen));
		axel::ui::on_set_screen(reinterpret_cast<CarsFrontEnd*>(_this), screen, reinterpret_cast<const char**>(ctx.ebp.unsigned_integer - 0x14));
	}
};

DefineReplacementHook(OncePerFrame) {
	static void __fastcall callback(void* _this) {
		if (SteamAPI_IsSteamRunning()) {
			axel::update_context();
		}
		original(_this);
	}
};

auto init_axel_ui_hooks() -> void {
	GetOptionsListHook::install_at_ptr(0x004be500);
	HandleForeignSetScreen::install_at_ptr(0x004c276f);
	CarsFrontEnd_UpdateHook::install_at_ptr(0x004bb1d0);
	OncePerFrame::install_at_ptr(0x0080d6c0);

	// Adds a new Online sub-menu to the main menu.
	static const char MAIN_MENU_OPTIONS_MENU_ITEMS[] = "FE_MM_StoryMissions,FE_MM_CustomMissions,FE_MM_Badges,FE_MM_Extras,FE_MM_Garage,FE_MM_Axel,Win32Wii_Scn_ExitToWindows";
	sunset::inst::push_u32(reinterpret_cast<void*>(0x004bf101), reinterpret_cast<std::uintptr_t>(&MAIN_MENU_OPTIONS_MENU_ITEMS));
	axel::init_context();
	axel::ingame::install_hooks();
}

#endif

extern "C" void __stdcall Pentane_Main() {
	// FIXME: link against Pentane.lib properly instead of this bullshit!!!!
	Pentane_LogUTF8 = reinterpret_cast<void(*)(PentaneCStringView*)>(GetProcAddress(GetModuleHandleA("Pentane.dll"), "Pentane_LogUTF8"));
	Pentane_IsWindowedModeEnabled = reinterpret_cast<int(*)()>(GetProcAddress(GetModuleHandleA("Pentane.dll"), "Pentane_IsWindowedModeEnabled"));

	// Make sure we're attached to Arcade, not any other game.
	const auto nt_header = reinterpret_cast<IMAGE_NT_HEADERS*>(reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr)) + reinterpret_cast<IMAGE_DOS_HEADER*>(GetModuleHandleW(nullptr))->e_lfanew);
	if (nt_header->FileHeader.TimeDateStamp != 0x521E2EAF) {
		logger::log("[ArcadeEssentials::Pentane_Main] `ArcadeEssentials` is not compatble with Cars 2: The Video Game (PC)!");
	}
	else {
		if (!GLOBAL_CONFIG->read()) {
			logger::log_format("[ArcadeEssentials::AEConfig::read] `settings.toml` is missing! VSync will be enabled, motion blur will be enabled, the game window will attempt to match 1280x720, and game progression will be disabled.");
			GLOBAL_CONFIG->finalize();
		}
		if (GLOBAL_CONFIG->vsync) {
			// Set the presentation interval to D3DPRESENT_INTERVAL_ONE if vertical sync is enabled in `settings.toml`.
			sunset::utils::set_permission(reinterpret_cast<void*>(0x008340f7 + 6), 4, sunset::utils::Perm::ExecuteReadWrite);
			*reinterpret_cast<std::uint32_t*>(0x008340f7 + 6) = D3DPRESENT_INTERVAL_ONE;
		}
		if (Pentane_IsWindowedModeEnabled() == 0) {
 			// Set the window dimensions to whatever the user set in `settings.toml`, if we're running in exclusive fullscreen.
			sunset::utils::set_permission(reinterpret_cast<void*>(0x00d70b51), 4, sunset::utils::Perm::ExecuteReadWrite);
			*reinterpret_cast<std::uint32_t*>(0x00d70b51) = static_cast<std::uint32_t>(GLOBAL_CONFIG->window_width);
			sunset::utils::set_permission(reinterpret_cast<void*>(0x00d70b58), 4, sunset::utils::Perm::ExecuteReadWrite);
			*reinterpret_cast<std::uint32_t*>(0x00d70b58) = static_cast<std::uint32_t>(GLOBAL_CONFIG->window_height);
		}
		if (GLOBAL_CONFIG->enable_progression) {
			// Fixes an issue where the game would constantly force all characters/modes to be unlocked on every `CarsFrontEnd::Update` call.
			sunset::inst::nop(reinterpret_cast<void**>(0x004bb200), 5);
		}
		if (GLOBAL_CONFIG->enable_intro_fmvs) {
			// Un-stubs the function responsible for queuing the original game's intro video playlist.
			CarsGame_PlayIntroVideos::install_at_ptr(0x005558f0);
		}
		if (!GLOBAL_CONFIG->enable_attract_fmvs) {
			// Prevents the game from sending you to the attract videos at the title screen.
			sunset::utils::set_permission(reinterpret_cast<void*>(0x004bc2c5), 1, sunset::utils::Perm::ExecuteReadWrite);
			*reinterpret_cast<char*>(0x004bc2c5) = 0xEB;
		}
		if (GLOBAL_CONFIG->enable_fast_nav) {
			// Stubs the `SetButtonDealy` ExternalInterface function to enable unrestricted menu navigation speed.
			sunset::inst::nop(reinterpret_cast<void*>(0x0116ac12), 0xC);
		}

		/* DEBUGGING HOOKS START */
#ifdef _DEBUG

		TransDebugHook::install_at_ptr(0x00cf6dd0);

		CallFlashFunction::install_at_ptr(0x01168710);
		
		// Allows multiple instances of the game.
		sunset::inst::jmp(reinterpret_cast<void*>(0x0080d650), reinterpret_cast<void*>(0x0080cf40));
		sunset::inst::nop(reinterpret_cast<void*>(0x0080dc53), 17);

		// Logs when any player is locked to a controller.
		LockPlayerToController::install_at_ptr(0x00816dd0);

		// init_message_logger_arcade();
#endif

#if defined(MP_STRATEGY_AXEL) || defined(_DEBUG)
		CarsFrontEnd_GoBack::install_at_ptr(0x004be200);
#endif
		/* DEBUGGING HOOKS END */

#ifdef MP_STRATEGY_AXEL
		init_axel_ui_hooks();
#endif

		// Change the window title from Octane2 Renderer Window -> Cars 2: Arcade
		char window_title[] = "Cars 2: Arcade";
		sunset::utils::set_permission(reinterpret_cast<void*>(0x0161dd9c), sizeof(window_title), sunset::utils::Perm::ReadWrite);
		std::memcpy(reinterpret_cast<void*>(0x0161dd9c), window_title, sizeof(window_title));

		// Allows the window's title bar to properly respond to system-wide Dark Mode.
		DarkModeWindowTitle::install_at_ptr(0x00834b08);

		// Set's ArcadeManager's initial VideoState to 16 (GAME_START), in order to force the game to skip all intro cutscenes.
		sunset::utils::set_permission(reinterpret_cast<void*>(0x004512ad), 1, sunset::utils::Perm::ExecuteReadWrite);
		*reinterpret_cast<char*>(0x004512ad) = 16;

		// Kills the 16s timer for the car select and track select screens.
		sunset::inst::nop(reinterpret_cast<void*>(0x004bc7d4), 4);
		sunset::inst::nop(reinterpret_cast<void*>(0x004bc4f3), 4);
		sunset::inst::nop(reinterpret_cast<void*>(0x004bbbfe), 4);
		sunset::inst::nop(reinterpret_cast<void*>(0x004bbd1f), 8);

		// Kills rendering calls for the Raw Thrills Char select timer and difficulty display on map select. BluedragonMask
		sunset::inst::nop(reinterpret_cast<void*>(0x00456a36), 5); // Timer
		sunset::inst::nop(reinterpret_cast<void*>(0x004bc5e2), 5); // Difficulty
		
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

		// Registers the otherwise-missing `GoBack` and `GetSpyPoints` callbacks inside CarsFrontEndFlashCallbacks::FrontendFlashFunctions::SetupFlashFunctions.
		RegisterMissingFlashFuncs::install_at_ptr(0x004c93e0);
		
		// Implements most of the logic for transitioning from screen to screen.
		OnConfirmHook::install_at_ptr(0x004be010);
		
		// Allows the game to transition from SaveFileLoading to MT_FrontEnd, as well as reset the volume to the user-configured values.
		sunset::inst::nop(reinterpret_cast<void*>(0x004c3b33), 0x33);
		OnSaveLoaded::install_at_ptr(0x004c3b33);
		
		// Maps the A/Cross instead of the START button to 'X', allowing menu navigation with A/Cross.
		sunset::inst::push_u8(reinterpret_cast<void*>(0x0116c2cc), static_cast<std::uint8_t>(ControllerButton::Cross));
		sunset::inst::push_u8(reinterpret_cast<void*>(0x01163fd0), static_cast<std::uint8_t>(ControllerButton::Cross));

		// Redirects the game's WinArcadeInputDriver to the otherwise-unused WindowsSystemInputDriver.
		sunset::inst::push_u32(reinterpret_cast<void*>(0x0080cf64), sizeof(WindowsSystemInputDriver)); // Patch argument to operator.new
		sunset::inst::call(reinterpret_cast<void*>(0x0080cf8d), reinterpret_cast<void*>(0x00814fa0)); // Replace call to constructor
		sunset::inst::call(reinterpret_cast<void*>(0x0080da31), WindowsSystemInputDriver_Initialize); // Replace call to ::Initialize member function

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
		sunset::inst::call(reinterpret_cast<void*>(0x004cd3e6), intercept_set_auto_man_drift);
		// Updates a local variable with the last set PersistentData value for AutoDrift_.
		SetAutoManDriftLocalVarFix::install_at_ptr(0x004cd3fc);
		// Fixes an issue where the UI doesn't update until the second X press.
		GetAutoManDriftFix::install_at_ptr(0x004cd4f7);

		// Un-stubs the function responsible for initializing the clearance level data and passing it over to Flash.
		sunset::inst::jmp(reinterpret_cast<void*>(0x004b8790), init_clearance_level_data);

		// Prevents the game from using ArcadeData's master volume on restarting an event.
		sunset::inst::nop(reinterpret_cast<void*>(0x004f75a7), 5);
		FixRestartVolume::install_at_ptr(0x004f75a7);
		// Completely stub ArcadeManager::SetVolume.
		sunset::utils::set_permission(reinterpret_cast<void*>(0x004515e0), 4, sunset::utils::Perm::ExecuteReadWrite);
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

		// Fixes an issue where finish times would be clamped to 180s.
		sunset::inst::nop(reinterpret_cast<void*>(0x00500a2a), 8);
		sunset::inst::nop(reinterpret_cast<void*>(0x00500baf), 8);
		sunset::inst::nop(reinterpret_cast<void*>(0x0050248d), 10);

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

		// Fixes an issue where ITZ would not kill in multiplayer.
		sunset::inst::nop(reinterpret_cast<void*>(0x006bf40a), 0x1A);

		// Fixes an issue where lemon cars would remain indefinetly `Invulnerable`.
		BeginFrameHook::install_at_ptr(0x006d432c);
		StartInvulnHook::install_at_ptr(0x006d5640);
		sunset::utils::set_permission(reinterpret_cast<void*>(0x006d3898), 1, sunset::utils::Perm::ExecuteReadWrite);
		*reinterpret_cast<std::uint8_t*>(0x006d3898) = 0;
		
		// Fixes an issue where non-race mission modes would always fall back to the same UI.
		InitHudElements::install_at_ptr(0x0055114c);
		InitHudElementsRace::install_at_ptr(0x00551054);
		sunset::inst::nop(reinterpret_cast<void*>(0x0055101b), 2);

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

		// Fixes an issue where the game would not correctly handle the `FlashMovieMultiInputEnabled` flash function.
		sunset::utils::set_permission(reinterpret_cast<void*>(0x0116A31B), 1, sunset::utils::Perm::ExecuteReadWrite);
		*reinterpret_cast<std::uint8_t*>(0x0116A31B) = 1;

		sunset::utils::set_permission(reinterpret_cast<void*>(0x004bc9c7), 1, sunset::utils::Perm::ExecuteReadWrite);
		*reinterpret_cast<std::uint8_t*>(0x004bc9c7) = 0xFF;

		sunset::utils::set_permission(reinterpret_cast<void*>(0x004b8772), 1, sunset::utils::Perm::ExecuteReadWrite);
		*reinterpret_cast<std::uint8_t*>(0x004b8772) = 0;

		// Modifies the input driver to continously search for controllers and handle instances where a controller drops/reconnects.
		sunset::inst::jmp(reinterpret_cast<void*>(0x00815fb0), WindowsSystemInputDriver_BeginInput);

		// Fixes an issue where Arcade would ignore the HudPosition_Multi flag in PlayerHud::Init.
		HandleHudPositionMulti::install_at_ptr(0x0054cf1a);

		// Removes the ability to toggle between exclusive fullscreen and windowed mode.
		// We do this because the original games' implementation is very error-prone.
		sunset::inst::nop(reinterpret_cast<void*>(0x00833268), 5);

		// Allows the FE_MM_WOC menu to appear in FrontEnd.
		sunset::inst::nop(reinterpret_cast<void*>(0x004bf0fa), 7);
		
		// Adds Win32Wii_Scn_ExitToWindows to the pause menu.
		static const char PAUSE_MENU_OPTIONS_LIST_WITH_RESTART[] = "IG_PA_Resume,IG_PA_Restart,IG_PA_Exit,Win32Wii_Scn_ExitToWindows";
		static const char PAUSE_MENU_OPTIONS_LIST[] = "IG_PA_Resume,IG_PA_Exit,Win32Wii_Scn_ExitToWindows";
		sunset::inst::mov_u32(reinterpret_cast<void*>(0x004d0309), sunset::inst::RegisterIndex::Eax, reinterpret_cast<std::uintptr_t>(&PAUSE_MENU_OPTIONS_LIST_WITH_RESTART));
		sunset::inst::mov_u32(reinterpret_cast<void*>(0x004d0302), sunset::inst::RegisterIndex::Eax, reinterpret_cast<std::uintptr_t>(&PAUSE_MENU_OPTIONS_LIST));		
		CheckForExitToWindows::install_at_ptr(0x004d0596);
		CheckForExitToWindows2::install_at_ptr(0x004d091c);

		// Adds a new Graphics sub-menu to the Options menu.
		static const char EXTRAS_OPTIONS_MENU_ITEMS[] = "FE_EX_Options,FE_EX_EnterCode,FE_EX_Credits,FE_EX_Graphics";
		sunset::inst::push_u32(reinterpret_cast<void*>(0x004bed9b), reinterpret_cast<std::uintptr_t>(&EXTRAS_OPTIONS_MENU_ITEMS));
		CheckForGraphicsMenu::install_at_ptr(0x004bee93);
		CheckForGraphicsMenu2::install_at_ptr(0x004cbcda);
		CheckForGraphicsMenu3::install_at_ptr(0x004c2394);
		ExternalInterfaceHandler_Callback::install_at_ptr(0x0116a080);

		// Prevents the game from multiplying camera distance values by 0.75.
		sunset::inst::nop(reinterpret_cast<void*>(0x0049e05c), 8);
		// Prevents the game from clamping the camera distance to 7.0.
		sunset::inst::nop(reinterpret_cast<void*>(0x0049e037), 5);

		// Changes the default camera distances.
		static const float FIVE = 5.0f;
		static const float NINE = 9.0f;
		static const float TEN = 10.0f;
		static const float FIFTEEN = 15.0f;
		sunset::utils::set_permission(reinterpret_cast<void*>(0x00489225 + 4), 4, sunset::utils::Perm::ExecuteReadWrite);
		*reinterpret_cast<std::uintptr_t*>(0x00489225 + 4) = reinterpret_cast<std::uintptr_t>(&FIVE);
		sunset::utils::set_permission(reinterpret_cast<void*>(0x0048a20c + 4), 4, sunset::utils::Perm::ExecuteReadWrite);
		*reinterpret_cast<std::uintptr_t*>(0x0048a20c + 4) = reinterpret_cast<std::uintptr_t>(&FIVE);
		sunset::utils::set_permission(reinterpret_cast<void*>(0x0048b92a + 4), 4, sunset::utils::Perm::ExecuteReadWrite);
		*reinterpret_cast<std::uintptr_t*>(0x0048b92a + 4) = reinterpret_cast<std::uintptr_t>(&NINE);
		sunset::utils::set_permission(reinterpret_cast<void*>(0x005a07a9 + 4), 4, sunset::utils::Perm::ExecuteReadWrite);
		*reinterpret_cast<std::uintptr_t*>(0x005a07a9 + 4) = reinterpret_cast<std::uintptr_t>(&TEN);
		sunset::utils::set_permission(reinterpret_cast<void*>(0x00482999 + 4), 4, sunset::utils::Perm::ExecuteReadWrite);
		*reinterpret_cast<std::uintptr_t*>(0x00482999 + 4) = reinterpret_cast<std::uintptr_t>(&TEN);
		sunset::utils::set_permission(reinterpret_cast<void*>(0x005a07ef + 4), 4, sunset::utils::Perm::ExecuteReadWrite);
		*reinterpret_cast<std::uintptr_t*>(0x005a07ef + 4) = reinterpret_cast<std::uintptr_t>(&FIFTEEN);
		FixDefaultDistance::install_at_ptr(0x0048a9cc);

		// Effectively removes the weird FOV scaling by setting the constant to 1.0f.
		sunset::utils::set_permission(reinterpret_cast<void*>(0x015f21cc), 4, sunset::utils::Perm::ReadWrite);
		*reinterpret_cast<std::uint32_t*>(0x015f21cc) = std::bit_cast<std::uint32_t, float>(1.0f);

		// Allows the game to choose the correct screen type value for CarTypeProperties.
		GetTypeCCI::install_at_ptr(0x0048b4b9);

		// Fixes an issue where UI elements (both 2D and 3D) would render at a higher-than-intended gamma.
		// I'm not nearly smart enough to know if this is really the "correct" way to do it. But I don't care, it works!
		FlashGuiImpRender::install_at_ptr(0x0116da70);

		// Forces the game to use XBOX360 `.dct` files instead of WIN32 ones.
		// FIXME: We might need to revert this later.
		sunset::utils::set_permission(reinterpret_cast<void*>(0x01647d44), 8, sunset::utils::Perm::ReadWrite);
		std::memcpy(reinterpret_cast<void*>(0x01647d44), "XBOX360", 8);	

		// Fixes UI scaling at resolutions higher than 720p.
		AdjustScaleformViewport::install_at_ptr(0x0116e28e);

		// Fixes an issue where the game thinks two players are active by default.
		sunset::inst::push_u8(reinterpret_cast<void*>(0x004c5c17), 1);

		// Fixes an issue where the mini-menu would omit the button labels on the main menu.
		MiniMenuAddButton::install_at_ptr(0x004b87a0);
		MiniMenuShowTite::install_at_ptr(0x004b89a0);
		MiniMenuResetButtons::install_at_ptr(0x004b88b0);

		// Prevents the game from removing the last mini-menu button on entering `MT_FrontEnd`.
		sunset::inst::nop(reinterpret_cast<void*>(0x004c179e), 5);

		// Replaces `TEMP_UNLOCK_BUTTONS`/`TEMP_UNLOCK_ALL` with `FrontEnd_NewGameButton`/`FrontEnd_New_Title`.
		static const char FRONTEND_NEW_GAME_BUTTON[] = "FrontEnd_NewGameButton";
		static const char FRONTEND_NEW_TITLE[] = "FrontEnd_New_Title";
		sunset::utils::set_permission(reinterpret_cast<void*>(0x004c2a1f + 6), 1, sunset::utils::Perm::ExecuteReadWrite);
		*reinterpret_cast<unsigned char*>(0x004c2a1f + 6) = std::to_underlying(CarsFrontEndScreen::SaveSlots);
		sunset::inst::push_u32(reinterpret_cast<void*>(0x004c2a28), reinterpret_cast<std::uintptr_t>(&FRONTEND_NEW_TITLE));
		sunset::inst::push_u32(reinterpret_cast<void*>(0x004c2a2d), reinterpret_cast<std::uintptr_t>(&FRONTEND_NEW_GAME_BUTTON));

		// Adds the default mini-menu buttons on initialization.
		AddButtons::install_at_ptr(0x004b86aa);

		// Removes the last mini-menu button as the game transitions from the `SaveSlots` screen to `MT_FrontEnd`
		CarsFrontEnd_SetScreen::install_at_ptr(0x004c1440);

		// Fixes an issue where the `Progress` and `JoinIn` FrontEnd components would dissappear at the wrong times.
		sunset::utils::set_permission(reinterpret_cast<void*>(0x004c14a1 + 3), 1, sunset::utils::Perm::ExecuteReadWrite);
		*reinterpret_cast<unsigned char*>(0x004c14a1 + 3) = 1;
		sunset::utils::set_permission(reinterpret_cast<void*>(0x004c14a5 + 3), 1, sunset::utils::Perm::ExecuteReadWrite);
		*reinterpret_cast<unsigned char*>(0x004c14a5 + 3) = 1;

		// Exposes engine support for more `VertexFormatType` values. 
		RendererConvertTo::install_at_ptr(0x00829520);

		// Prevents the game from adding the `Scn_Download` button to the mini-menu while hovering over DLC characters in the Garage.
		sunset::inst::nop(reinterpret_cast<void*>(0x004ccd93), 10);
		sunset::inst::nop(reinterpret_cast<void*>(0x004ccda3), 5);

		// Scales the Picture-In-Picture textures depending on the current resolution.
		SetPIPWidth::install_at_ptr(0x004ecfca);
		SetPIPHeight::install_at_ptr(0x004ecff5);

		// Sets the resolution of runtime shadow-map textures to their user-configured values.
		SetShadowResolution::install_at_ptr(0x0062ec0d);
		SetEnableHiResShadows::install_at_ptr(0x006336ef);
		SetEnableHiResShadows2::install_at_ptr(0x0062ed05);

		// Allows the user to toggle on/off Motion Blur.
		SetMotionBlurHook::install_at_ptr(0x0062b868);
		
		// Fixes an issue where certain stats would not track in the global `ChallengeManager` database.
		FixChallengeManagerUnk::install_at_ptr(0x00530af0);
		
		install_multiplayer_hooks();

		// Prevent the game from pseudo-pausing when the ALT key is pressed.
		sunset::inst::nop(reinterpret_cast<void*>(0x00833279), 0x12);

		// Restores the default lap count from 1 -> 3.
		sunset::inst::nop(reinterpret_cast<void*>(0x004e8382), 6);
		sunset::inst::nop(reinterpret_cast<void*>(0x004e838e), 6);
		sunset::utils::set_permission(reinterpret_cast<void*>(0x004e6b13 + 6), sizeof(std::uint32_t), sunset::utils::Perm::ExecuteReadWrite);
		*reinterpret_cast<std::uint32_t*>(0x004e6b13 + 6) = 3;
		sunset::utils::set_permission(reinterpret_cast<void*>(0x004e6b20 + 6), sizeof(std::uint32_t), sunset::utils::Perm::ExecuteReadWrite);
		*reinterpret_cast<std::uint32_t*>(0x004e6b20 + 6) = 3;
		
		// Stub QR code rendering logic in `r_execute` to fix an issue where the Language Select menu wouldn't render.
		sunset::inst::nop(reinterpret_cast<void*>(0x00872fc3), 5);

		// Re-enables the Language Select menu.
		sunset::inst::nop(reinterpret_cast<void*>(0x00eb38bf), 2);

		// Fixes an issue where all Light-class cars would be forced to Medium.
		sunset::inst::nop(reinterpret_cast<void*>(0x004dc9d8), 5);

		// Fixes an issue where all car masses would be clamped to 400.0.
		sunset::inst::nop(reinterpret_cast<void*>(0x004dca1c), 8);

		// Fixes an issue where tire widths would be clamped to 0.3.
		sunset::inst::nop(reinterpret_cast<void*>(0x004dca89), 8);
		sunset::inst::nop(reinterpret_cast<void*>(0x004dcaaa), 8);

		// SkateJack reticule fix at higher/lower resolutions than 1280x720. (alongside aspect ratio)
		HudSkateReticuleHook::install_at_ptr(0x00534890);

		// Inverts the steering input when backwards-driving.
		BackwardSteeringInvert::install_at_ptr(0x006a4e30);
		sunset::inst::nop(reinterpret_cast<void*>(0x006b0af3), 2);

		// Swaps the leaning and eye animations during backwards-driving.
		FixBwdEyeL::install_at_ptr(0x006a56d2);
		FixBwdEyeR::install_at_ptr(0x006a571a);
		FixBwdLean::install_at_ptr(0x006a5793);

		DisplayWeaponTutorialHook::install_at_ptr(0x005c1a30);
		MiniMenuSwapFirstButton::install_at_ptr(0x004b8830);
		FixFireButtonUI::install_at_ptr(0x005ada9b);
		FixRevItButtonUI::install_at_ptr(0x005451a9);
		FixHudTutorialButtonPrompt::install_at_ptr(0x00538ddb);
		sunset::inst::nop(reinterpret_cast<void*>(0x004cd5ae), 0xD);
		FixAutoDriftButtonUI::install_at_ptr(0x004cd5ae);
		FixLoadTipUI::install_at_ptr(0x00559e96);

		// Forces the game to always use `ui/controllerbuttons.oct` instead of switching to `ui/tutorialbuttons.oct`.
		sunset::utils::set_permission(reinterpret_cast<void*>(0x00551663), 1, sunset::utils::Perm::ExecuteReadWrite);
		*reinterpret_cast<std::uint8_t*>(0x00551663) = 0xEB;

		// Fixes an issue where `ControllerFocusManager::GetCurrent` would always return nullptr, and thus allows the game to update control mappers.
		// Allows us to change UI binds on the fly in `FlashControlMapper::Update`. Has the unintended side effected of making menu navigation slow again. Will not be fixing this unless bluedragon does it for me.
		sunset::utils::set_permission(reinterpret_cast<void*>(0x008115fc), 1, sunset::utils::Perm::ExecuteReadWrite);
		*reinterpret_cast<std::uint8_t*>(0x008115fc) = 0x7E;

		FlashControlMapperUpdateHook::install_at_ptr(0x01164190);
		FlashInputActionMapHook::install_at_ptr(0x0116c4ce);
		CCMSetPlayerID::install_at_ptr(0x00661140);
		FlashToDevonHook::install_at_ptr(0x0116af20);
		CarsFrontEnd_ShowControllerPull_HandleMessage::install_at_ptr(0x004c5c50);
		ErrorPopup_HideControllerPullMessage_HandleMessage::install_at_ptr(0x00e9fef0);

		install_fmv_driver();

		logger::log("[ArcadeEssentials::Pentane_Main] Installed hooks!");
	} 
}
