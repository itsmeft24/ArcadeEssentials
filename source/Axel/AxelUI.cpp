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

auto axel::ui::update(CarsFrontEnd* _this) -> void {

	if (axel::received_lobby_list()) {
		std::vector<CSteamID> lobbies = axel::consume_lobby_list();

		// TODO: Send the user to a menu showing all open lobbies. For now, we can just change the text in the popup and wait for us to join the lobby.
		std::uintptr_t* inst = *g_ErrorPopup;
		auto _SetTitle = *reinterpret_cast<bool(__thiscall**)(std::uintptr_t*, const char*)>(*inst + 0x20);
		auto _SetDescText = *reinterpret_cast<bool(__thiscall**)(std::uintptr_t*, const char*, int)>(*inst + 0x24);
		_SetTitle(inst, "Axel_Msg_JoiningLobby");
		_SetDescText(inst, "Axel_Msg_WaitingForServer", 1);

		axel::request_join(lobbies[0]);
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
}

auto axel::ui::on_confirm(CarsFrontEnd* _this, std::string_view selected_menu) -> void
{
	CarsFrontEndScreen screen = *reinterpret_cast<CarsFrontEndScreen*>(reinterpret_cast<std::uintptr_t>(_this) + 0xA8);
	if (screen == CarsFrontEndScreen::Axel_Online) {
		// Enable Axel.
		axel::set_online(true);

		// If we selected `Axel_LobbyCreate`, send the user to `Axel_LobbyCreateOptions` to configure their lobby settings.
		if (selected_menu == "Axel_LobbyCreate") {
			logger::log_format("[CarsFrontEnd::OnConfirm] Sending to: `Axel_LobbyCreateOptions`.");
			_CarsFrontEnd_SetScreen(_this, CarsFrontEndScreen::Axel_LobbyCreateOptions, selected_menu.data(), true);
		}
		// If we selected `Axel_LobbyJoin`, send the user to `Axel_LobbyCreateOptions` to configure their lobby settings.
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

			_CarsFrontEnd_SetGameModeIndex(_this, "FE_MT_Race");
			(*g_GameProgressionManager)->m_unkIndex = -1;
			_CarsFrontEnd_SetLevelAndUnk(_this, "TRACK_A_OilRig");
			_CarsFrontEnd_UnkHandleTrackLengthType(_this, "TRACK_A_OilRig");
			(*g_GameProgressionManager)->SetAICarCount(0);
			GameProgressionManager_SetAICarCount(*g_GameProgressionManager, 0);
			GameProgressionManager_SetMissionTimeByMode(*g_GameProgressionManager, false);
		}
	}
	else if (screen == CarsFrontEndScreen::Axel_LobbyCreateOptions) {
		if (selected_menu.contains("Axel_LobbyOption")) {

			// TODO: Wrap this in `axel::`.
			SteamMatchmaking()->CreateLobby(k_ELobbyTypePublic, 10);

			axel::CONTEXT->myAxelId = 0;
			axel::CONTEXT->playerCount = 1;
			axel::CONTEXT->isHost = true;
			axel::CONTEXT->lobbyMembers[0].playerId = 0;
			axel::CONTEXT->lobbyMembers[0].steamId = SteamUser()->GetSteamID();

			_CarsFrontEnd_SetGameModeIndex(_this, "FE_MT_Race");
			*reinterpret_cast<int*>(*g_GameProgressionManager + 0xB8) = -1;
			_CarsFrontEnd_SetLevelAndUnk(_this, "TRACK_A_OilRig");
			_CarsFrontEnd_UnkHandleTrackLengthType(_this, "TRACK_A_OilRig");
			GameProgressionManager_SetAICarCount(*g_GameProgressionManager, 0);
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
}

auto axel::ui::retrieve_options_list(CarsFrontEnd* _this, CarsFrontEndScreen screen) -> std::optional<std::string> {
	if (screen == CarsFrontEndScreen::Axel_Online) {
		return "Axel_LobbyCreate,Axel_LobbyJoin";
	}
	else if (screen == CarsFrontEndScreen::Axel_LobbyCreateOptions) {
		return "Axel_LobbyOptionName,Axel_LobbyOptionHasPassword,Axel_LobbyOptionPassword";
	}
	return std::nullopt;
}
