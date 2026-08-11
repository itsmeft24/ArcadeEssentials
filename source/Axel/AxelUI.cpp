#include "sunset.hpp"
#include "../pentane.hpp"
#include "../Game/GameProgressionManager.hpp"
#include "../Game/MiscOffsets.hpp"
#include "../Patch/Input/WindowsSystemInputDriver.hpp"
#include "Axel.hpp"

static bool SHOW_RIGHT = false;
Flash::Movie* RIGHT_SIDE_CSS = nullptr;

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

DefineReplacementHook(SoftwareKeyboardHook) {
	static void __fastcall callback(void* _this, std::uintptr_t edx, const char** param, Flash::Movie * movie) {
		if ((*g_FrontEnd)->current.screen != CarsFrontEndScreen::Axel_EnterLobbyName) {
			original(_this, edx, param, movie);
			return;
		}

		std::string entered = *param;

		if (entered == "/") {
			_CarsFrontEnd_OnConfirm(*g_FrontEnd, "", nullptr);
			return;
		}

		if (entered == "&#94;" || entered == "^") {
			axel::CONTEXT->softwareKeyboardCapsLock = !axel::CONTEXT->softwareKeyboardCapsLock;
			return;
		}
		if (entered == "&lt;" || entered == "<") {
			if (axel::CONTEXT->softwareKeyboardBuffer.length() >= 1) {
				axel::CONTEXT->softwareKeyboardBuffer.pop_back();
			}
			movie->CallFlashFunction("SetKeyCode", nullptr, axel::CONTEXT->softwareKeyboardBuffer.data(), 0, 0.0f);
			return;
		}

		if (axel::CONTEXT->softwareKeyboardBuffer.length() >= 30) {
			return;
		}

		if (entered == "_") {
			entered = " ";
		}
		else if (axel::CONTEXT->softwareKeyboardCapsLock) {
			entered = std::toupper(entered[0]);
		}

		axel::CONTEXT->softwareKeyboardBuffer += entered;

		movie->CallFlashFunction("SetKeyCode", nullptr, axel::CONTEXT->softwareKeyboardBuffer.data(), 0, 0.0f);
	}
};

DefineInlineHook(RightSideCSSHandleInputHook) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		bool shouldHandle = (ctx.eax.unsigned_integer & 0xFF) != 0;
		if ((*g_FrontEnd)->current.screen == CarsFrontEndScreen::CarSelect) {
			if ((*g_InputPtr)->GetController(0)->ButtonPressed(ControllerButton::Triangle)) {
				SHOW_RIGHT = !SHOW_RIGHT;
			}
			shouldHandle = shouldHandle && !SHOW_RIGHT;
		}
		ctx.eax.unsigned_integer = shouldHandle;

		if (SHOW_RIGHT) {
			RIGHT_SIDE_CSS->HandleInput(false, 0);
		}
	}
};

DefineReplacementHook(SwapCharacter) {
	static void __fastcall callback(void* _this, std::uintptr_t edx, const char** args, Flash::Movie * movie) {
		const char* _str = args[0];
		int index = _str[0] - '0';
		if (index == 0) {
			const char* carId = _str + 1;
#if MP_STRATEGY_AXEL
			if (axel::online() && index == 0) {
				SteamMatchmaking()->SetLobbyMemberData(axel::CONTEXT->lobbyID, "selected_car", carId);
			}
#endif
		}
		return original(_this, edx, args, movie);
	}
};

auto axel::ui::install_hooks() -> void {
	GetOptionsListHook::install_at_ptr(0x004be500);
	HandleForeignSetScreen::install_at_ptr(0x004c276f);
	CarsFrontEnd_UpdateHook::install_at_ptr(0x004bb1d0);
	OncePerFrame::install_at_ptr(0x0080d6c0);

	// Adds a new Online sub-menu to the main menu.
	static const char MAIN_MENU_OPTIONS_MENU_ITEMS[] = "FE_MM_StoryMissions,FE_MM_CustomMissions,FE_MM_Badges,FE_MM_Extras,FE_MM_Garage,FE_MM_Axel,Win32Wii_Scn_ExitToWindows";
	sunset::inst::push_u32(reinterpret_cast<void*>(0x004bf101), reinterpret_cast<std::uintptr_t>(&MAIN_MENU_OPTIONS_MENU_ITEMS));

	SoftwareKeyboardHook::install_at_ptr(0x004cb7d0);
	RightSideCSSHandleInputHook::install_at_ptr(0x004bc96d);

	SwapCharacter::install_at_ptr(0x004cd0f0);

	// Hack that allows the player to be sent directly to the CSS after an event.
	sunset::inst::nop(reinterpret_cast<void*>(0x004c0db5), 2);
}

auto axel::ui::on_retract(CarsFrontEnd* _this) -> void {
	// If we back out of the `Axel_Online` screen, we need to disable Axel entirely.
	CarsFrontEndScreen screen = *reinterpret_cast<CarsFrontEndScreen*>(reinterpret_cast<std::uintptr_t>(_this) + 0xA8);
	if (screen == CarsFrontEndScreen::Axel_Online) {
		axel::set_online(false);
	}

	// If we back out of the CSS, we need to leave whatever lobby we're in.
	if (screen == CarsFrontEndScreen::CarSelect) {
		if (axel::CONTEXT->inLobby) {
			axel::leave_lobby();
			(*g_FlashGui)->RemoveMovie(&RIGHT_SIDE_CSS, false);
			RIGHT_SIDE_CSS = nullptr;
		}
	}
}

auto axel::ui::update(CarsFrontEnd* _this) -> void {

	if (axel::CONTEXT->noOpenLobbiesSendBack) {
		axel::CONTEXT->noOpenLobbiesPopupTimerMs -= *reinterpret_cast<int*>(0x01925e30);
		if (axel::CONTEXT->noOpenLobbiesPopupTimerMs <= 0) {
			ErrorPopup_MarkForClose(*g_ErrorPopup);
			_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::Axel_Online, nullptr, false);
			axel::CONTEXT->noOpenLobbiesSendBack = false;
		}
	}

	if (axel::received_lobby_list()) {
		std::vector<CSteamID> lobbies = axel::consume_lobby_list();

		if (lobbies.size() == 0) {
			std::uintptr_t* inst = *g_ErrorPopup;
			auto _SetTitle = *reinterpret_cast<bool(__thiscall**)(std::uintptr_t*, const char*)>(*inst + 0x20);
			auto _SetDescText = *reinterpret_cast<bool(__thiscall**)(std::uintptr_t*, const char*, int)>(*inst + 0x24);
			auto _ShowPopup = *reinterpret_cast<bool(__thiscall**)(std::uintptr_t*, int)>(*inst + 0x14);
			auto _CloseAndShowPopup = *reinterpret_cast<bool(__thiscall**)(std::uintptr_t*, int)>(*inst + 0x18);
			auto _SetOptions = *reinterpret_cast<bool(__thiscall**)(std::uintptr_t*, const char*)>(*inst + 0x1C);
			_SetTitle(inst, "Popup_Title_Attention");
			_SetDescText(inst, "Axel_NoOpenLobbies", 1);
			_SetOptions(inst, "SharedText_OK");
			_CloseAndShowPopup(inst, 1);
			axel::CONTEXT->noOpenLobbiesSendBack = true;
			axel::CONTEXT->noOpenLobbiesPopupTimerMs = 700;
		}
		else {
			ErrorPopup_MarkForClose(*g_ErrorPopup);
			_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::Axel_LobbyShowList, nullptr, true);
		}
	}

	if (axel::has_joined_lobby()) {
		ErrorPopup_MarkForClose(*g_ErrorPopup);
		axel::CONTEXT->lobbyJoinRequested = false;
		axel::CONTEXT->lobbyJoinRequestFulfilled = false;

		// Send user to character selection screen.
		PersistentData_SetGlobal(*g_PersistentData, "NumPlayers", axel::player_count());
		PersistentData_SetGlobal(*g_PersistentData, "MultiPlayer", 1);
		PersistentData_SetGlobal(*g_PersistentData, "ScreenFormat", 1);
		_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::CarSelect, nullptr, true);

		std::string key = "menu_button_extra1";
		key += SuffixForCT((*g_InputPtr)->GetController(0)->m_uiType);
		_this->miniMenu->movie->CallFlashFunction("AddButton", nullptr, key.data(), "Axel_SwapSelection");
	}

	if (axel::has_created_lobby()) {
		ErrorPopup_MarkForClose(*g_ErrorPopup);
		axel::CONTEXT->lobbyCreateRequested = false;
		axel::CONTEXT->lobbyCreated = false;

		// Send user to character selection screen.
		PersistentData_SetGlobal(*g_PersistentData, "NumPlayers", axel::player_count());
		PersistentData_SetGlobal(*g_PersistentData, "MultiPlayer", 1);
		PersistentData_SetGlobal(*g_PersistentData, "ScreenFormat", 1);
		_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::CarSelect, nullptr, false);

		std::string key = "menu_button_extra1";
		key += SuffixForCT((*g_InputPtr)->GetController(0)->m_uiType);
		_this->miniMenu->movie->CallFlashFunction("AddButton", nullptr, key.data(), "Axel_SwapSelection");

		SteamMatchmaking()->SetLobbyData(axel::CONTEXT->lobbyID, "name", axel::CONTEXT->softwareKeyboardBuffer.data());
		SteamMatchmaking()->SetLobbyData(axel::CONTEXT->lobbyID, "is_axel", "true");
	}

	if (axel::CONTEXT->playerCountChanged) {
		axel::ui::refresh_lobby_member_list();
		axel::CONTEXT->playerCountChanged = false;
	}

	if (axel::CONTEXT->selectedCarChanged) {
		if (RIGHT_SIDE_CSS != nullptr) {
			RIGHT_SIDE_CSS->CallFlashFunction("RefreshSelectedCharacter", nullptr);
		}
		axel::CONTEXT->selectedCarChanged = false;
	}
}

auto axel::ui::on_confirm(CarsFrontEnd* _this, std::string_view selected_menu) -> void {
	CarsFrontEndScreen screen = *reinterpret_cast<CarsFrontEndScreen*>(reinterpret_cast<std::uintptr_t>(_this) + 0xA8);
	logger::log_format("[axel::ui::on_confirm] {}", selected_menu);

	if (screen == CarsFrontEndScreen::Axel_Online) {
		// Enable Axel.
		axel::set_online(true);

		// If we selected `Axel_LobbyCreate`, send the user to `Axel_EnterLobbyName` to set the lobby name.
		// NOTE: I don't know how to get the keyboard to link up with the settings page, so we have to ditch that idea for now.
		if (selected_menu == "Axel_LobbyCreate") {
			logger::log_format("[CarsFrontEnd::OnConfirm] Sending to: `Axel_EnterLobbyName`.");
			_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::Axel_EnterLobbyName, selected_menu.data(), true);
			axel::CONTEXT->softwareKeyboardBuffer.clear();
		}
		// If we selected `Axel_LobbyJoin`, request the lobby list, then show a popup while we wait for the lobby list to come back.
		else if (selected_menu == "Axel_LobbyJoin") {
			axel::request_lobby_list();
			// This will ask the lobby service for the lobby list, and tag the axel::Context
			// with a bool denoting that we're waiting for a result. Then, in `axel::ui::update`,
			// we can check if we're:
			// 1. waiting for the lobby service to give us the list
			// 2. have recieved the list.
			// If those two conditions are satisfied, we can then close the popup.

			// Creates a popup with the `Axel_Msg_FetchingLobbies` title.
			// In-game, this looks like the same popup used when loading from a save file or saving your game.
			std::uintptr_t* inst = *g_ErrorPopup;
			auto _SetTitle = *reinterpret_cast<bool(__thiscall**)(std::uintptr_t*, const char*)>(*inst + 0x20);
			auto _SetDescText = *reinterpret_cast<bool(__thiscall**)(std::uintptr_t*, const char*, int)>(*inst + 0x24);
			auto _ShowPopup = *reinterpret_cast<bool(__thiscall**)(std::uintptr_t*, int)>(*inst + 0x14);
			_SetTitle(inst, "Axel_Msg_FetchingLobbies");
			_SetDescText(inst, "Axel_Msg_WaitingForServer", 1);
			_ShowPopup(inst, 2);

			_CarsFrontEnd_SetGameType(_this, "FE_MT_BattleRace");
			(*g_GameProgressionManager)->m_unkIndex = -1;
			(*g_GameProgressionManager)->m_settings.m_weaponsEnabled = true;
			(*g_GameProgressionManager)->m_settings.m_friendlyFireType = 1;
			_CarsFrontEnd_SetLevelAndUnk(_this, "TRACK_A_RadiatorSprings");
			_CarsFrontEnd_UnkHandleTrackLengthType(_this, "TRACK_A_RadiatorSprings");
			(*g_GameProgressionManager)->SetAICarCount(0);
			(*g_GameProgressionManager)->m_settings.m_lapCount = 1;
			(*g_GameProgressionManager)->m_settings.m_maxLapCount = 1;
			GameProgressionManager_SetMissionTimeByMode(*g_GameProgressionManager, false);
		}
	}
	else if (screen == CarsFrontEndScreen::Axel_LobbyShowList) {
		int index = std::stoi(selected_menu.data() + 10);

		std::uintptr_t* inst = *g_ErrorPopup;
		auto _SetTitle = *reinterpret_cast<bool(__thiscall**)(std::uintptr_t*, const char*)>(*inst + 0x20);
		auto _SetDescText = *reinterpret_cast<bool(__thiscall**)(std::uintptr_t*, const char*, int)>(*inst + 0x24);
		_SetTitle(inst, "Axel_Msg_JoiningLobby");
		_SetDescText(inst, "Axel_Msg_WaitingForServer", 1);

		axel::request_join(CONTEXT->lobbyList[index]);
	}
	else if (screen == CarsFrontEndScreen::Axel_EnterLobbyName) {
		logger::log_format("[axel::ui::on_confirm] Set Lobby Name: {}", axel::CONTEXT->softwareKeyboardBuffer);
		_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::Axel_LobbyCreateOptions, nullptr, false);
	}
	else if (screen == CarsFrontEndScreen::Axel_LobbyCreateOptions) {
		if (selected_menu.contains("Axel_LobbyOption")) {
			axel::request_create();

			axel::CONTEXT->myAxelId = 0;
			axel::CONTEXT->playerCount = 1;
			axel::CONTEXT->isHost = true;
			axel::CONTEXT->lobbyMembers[0].playerId = 0;
			axel::CONTEXT->lobbyMembers[0].steamId = SteamUser()->GetSteamID();

			_CarsFrontEnd_SetGameType(_this, "FE_MT_BattleRace");
			(*g_GameProgressionManager)->m_unkIndex = -1;
			(*g_GameProgressionManager)->m_settings.m_weaponsEnabled = true;
			(*g_GameProgressionManager)->m_settings.m_friendlyFireType = 1;
			_CarsFrontEnd_SetLevelAndUnk(_this, "TRACK_A_RadiatorSprings");
			_CarsFrontEnd_UnkHandleTrackLengthType(_this, "TRACK_A_RadiatorSprings");
			(*g_GameProgressionManager)->SetAICarCount(0);
			(*g_GameProgressionManager)->m_settings.m_lapCount = 1;
			(*g_GameProgressionManager)->m_settings.m_maxLapCount = 1;
			GameProgressionManager_SetMissionTimeByMode(*g_GameProgressionManager, false);
		}
	}
}

auto axel::ui::on_set_screen(CarsFrontEnd* _this, CarsFrontEndScreen screen, const char** outSwf) -> void {
	if (screen == CarsFrontEndScreen::Axel_Online) {
		*outSwf = "axel_online";
	}
	else if (screen == CarsFrontEndScreen::Axel_LobbyCreateOptions) {
		*outSwf = "axel_lobby_create_settings";
	}
	else if (screen == CarsFrontEndScreen::Axel_LobbyShowList) {
		*outSwf = "axel_lobby_list"; // "frnt_end_custom_squad_series";
	}
	else if (screen == CarsFrontEndScreen::Axel_EnterLobbyName) {
		*outSwf = "axel_util_swkbd";
	}
}

auto axel::ui::retrieve_options_list(CarsFrontEnd* _this, CarsFrontEndScreen screen) -> std::optional<std::string> {
	if (screen == CarsFrontEndScreen::Axel_Online) {
		return "Axel_LobbyCreate,Axel_LobbyJoin";
	}
	else if (screen == CarsFrontEndScreen::Axel_LobbyCreateOptions) {
		return "Axel_LobbyOptionName,Axel_LobbyOptionHasPassword,Axel_LobbyOptionPassword";
	}
	else if (screen == CarsFrontEndScreen::Axel_LobbyShowList) {
		std::string lobbyListStr = "";
		for (auto i = 0; i < axel::CONTEXT->lobbyList.size(); i++) {
			lobbyListStr += std::format("Axel_Lobby{},", i);
		}
		if (!lobbyListStr.empty())
			lobbyListStr.pop_back();
		return lobbyListStr;
	}/*
	else if (screen == CarsFrontEndScreen::CarSelect) {
		std::string lobbyListStr = "";
		for (auto i = 0; i < axel::CONTEXT->playerCount; i++) {
			lobbyListStr += std::format("Axel_LobbyMember{},", i);
		}
		if (!lobbyListStr.empty())
			lobbyListStr.pop_back();
		return lobbyListStr;
	}
	*/
	return std::nullopt;
}

auto axel::ui::get_lobby_descriptions() -> std::string {
	return "Race^Has Password|No^Laps|3^Weapons|Machine Guns";
}

auto axel::ui::get_lobby_name(int i) -> std::string {
	return SteamMatchmaking()->GetLobbyData(CONTEXT->lobbyList[i], "name");
}

auto axel::ui::load_lobby_screen_right() -> void {
	RIGHT_SIDE_CSS = (*g_FlashGui)->AddMovie("axel_lobby_screen_right", true, 0, 0);
	RIGHT_SIDE_CSS->SetImageLoader(&(*g_FrontEnd)->unknown3);
}

auto axel::ui::refresh_lobby_member_list() -> void {
	if (RIGHT_SIDE_CSS != nullptr) {
		RIGHT_SIDE_CSS->CallFlashFunction("RefreshLobbyMemberList", nullptr);
	}
}