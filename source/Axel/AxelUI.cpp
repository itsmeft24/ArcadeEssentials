#include "Axel.hpp"
#include "../pentane.hpp"
#include "../Game/GameProgressionManager.hpp"
#include "../Game/MiscOffsets.hpp"
#include "../Patch/Input/WindowsSystemInputDriver.hpp"

auto axel::ui::on_retract(CarsFrontEnd* _this) -> void {
	// If we back out of the `Axel_Online` screen, we need to disable Axel entirely.
	CarsFrontEndScreen screen = *reinterpret_cast<CarsFrontEndScreen*>(reinterpret_cast<std::uintptr_t>(_this) + 0xA8);
	if (screen == CarsFrontEndScreen::Axel_Online) {
		axel::set_online(false);
	}
}

extern std::string SWKBD_BUFFER;

#define KICK_YOU_OUT_IF_NO_LOBBIES_EXIST

#ifdef KICK_YOU_OUT_IF_NO_LOBBIES_EXIST
bool sendBack = false;
int sendBackTimerMs = 0;
#endif // KICK_YOU_OUT_IF_NO_LOBBIES_EXIST

auto axel::ui::update(CarsFrontEnd* _this) -> void {

#ifdef KICK_YOU_OUT_IF_NO_LOBBIES_EXIST
	if (sendBack) {
		sendBackTimerMs -= *reinterpret_cast<int*>(0x01925e30);
		if (sendBackTimerMs <= 0) {
			ErrorPopup_MarkForClose(*g_ErrorPopup);
			_CarsFrontEnd_GoBack(_this);
			sendBack = false;
		}
	}
#endif // KICK_YOU_OUT_IF_NO_LOBBIES_EXIST

	if (axel::received_lobby_list()) {
		std::vector<CSteamID> lobbies = axel::consume_lobby_list();

#ifdef KICK_YOU_OUT_IF_NO_LOBBIES_EXIST

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
			sendBack = true;
			sendBackTimerMs = 700;
		}
		else {
			ErrorPopup_MarkForClose(*g_ErrorPopup);
			_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::Axel_LobbyShowList, nullptr, true);
		}
#endif
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
	}

	if (axel::has_created_lobby()) {
		ErrorPopup_MarkForClose(*g_ErrorPopup);
		axel::CONTEXT->lobbyCreateRequested = false;
		axel::CONTEXT->lobbyCreated = false;

		// Send user to character selection screen.
		PersistentData_SetGlobal(*g_PersistentData, "NumPlayers", axel::player_count());
		PersistentData_SetGlobal(*g_PersistentData, "MultiPlayer", 1);
		PersistentData_SetGlobal(*g_PersistentData, "ScreenFormat", 1);
		_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::CarSelect, nullptr, true);

		SteamMatchmaking()->SetLobbyData(axel::CONTEXT->lobbyID, "name", SWKBD_BUFFER.data());
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

			_CarsFrontEnd_SetGameModeIndex(_this, "FE_MT_BattleRace");
			(*g_GameProgressionManager)->m_unkIndex = -1;
			(*g_GameProgressionManager)->m_settings.m_weaponsEnabled = true;
			(*g_GameProgressionManager)->m_settings.m_friendlyFireType = 1;
			_CarsFrontEnd_SetLevelAndUnk(_this, "Track_A_RadiatorSprings");
			_CarsFrontEnd_UnkHandleTrackLengthType(_this, "Track_A_RadiatorSprings");
			(*g_GameProgressionManager)->SetAICarCount(0);
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
		logger::log_format("[axel::ui::on_confirm] Set Lobby Name: {}", SWKBD_BUFFER);
		_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::Axel_LobbyCreateOptions, selected_menu.data(), true);
	}
	else if (screen == CarsFrontEndScreen::Axel_LobbyCreateOptions) {
		if (selected_menu.contains("Axel_LobbyOption")) {
			axel::request_create();

			axel::CONTEXT->myAxelId = 0;
			axel::CONTEXT->playerCount = 1;
			axel::CONTEXT->isHost = true;
			axel::CONTEXT->lobbyMembers[0].playerId = 0;
			axel::CONTEXT->lobbyMembers[0].steamId = SteamUser()->GetSteamID();

			_CarsFrontEnd_SetGameModeIndex(_this, "FE_MT_BattleRace");
			(*g_GameProgressionManager)->m_unkIndex = -1;
			(*g_GameProgressionManager)->m_settings.m_weaponsEnabled = true;
			(*g_GameProgressionManager)->m_settings.m_friendlyFireType = 1;
			_CarsFrontEnd_SetLevelAndUnk(_this, "Track_A_RadiatorSprings");
			_CarsFrontEnd_UnkHandleTrackLengthType(_this, "Track_A_RadiatorSprings");
			(*g_GameProgressionManager)->SetAICarCount(0);
			GameProgressionManager_SetMissionTimeByMode(*g_GameProgressionManager, false);
			
			// Send user to character selection screen.
			PersistentData_SetGlobal(*g_PersistentData, "NumPlayers", 1);
			PersistentData_SetGlobal(*g_PersistentData, "MultiPlayer", 1);
			PersistentData_SetGlobal(*g_PersistentData, "ScreenFormat", 1);
			_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::CarSelect, nullptr, true);
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
	}
	return std::nullopt;
}

auto axel::ui::get_lobby_descriptions() -> std::string {
	return "Race^Has Password|No^Laps|3^Weapons|Machine Guns";
}

auto axel::ui::get_lobby_name(int i) -> std::string {
	return SteamMatchmaking()->GetLobbyData(CONTEXT->lobbyList[i], "name");
}
