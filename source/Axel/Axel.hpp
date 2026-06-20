#pragma once
#include <optional>
#include <string>
#include <string_view>
#include <chrono>
#include "AxelContext.hpp"
#include "../Game/FrontEnd/CarsFrontEnd.hpp"

using namespace std::chrono_literals;

namespace axel {

	// How many updates are sent/received every second.
	constexpr auto TICK_RATE_HZ = 60;

	// The time between each sent/received update.
	constexpr auto TICK_TIME = 16.6666666666667ms;

	// Updates the global context with respect to the UI system. Called every `CarsFrontEnd::Update`.
	namespace ui {
		// Called every `CarsFrontEnd::GoBack`.
		auto on_retract(CarsFrontEnd* _this) -> void;

		// Called every `CarsFrontEnd::Update`.
		auto update(CarsFrontEnd* _this) -> void;

		// Called on every `CarsFrontEnd::Update` for which a foreign (non-vanilla) screen state is observed.
		auto on_confirm(CarsFrontEnd* _this, std::string_view selected_menu) -> void;

		// Called on every `CarsFrontEnd::SetScreen` for which a foreign (non-vanilla) screen state is observed.
		auto on_set_screen(CarsFrontEnd* _this, CarsFrontEndScreen screen, const char** outSwf) -> void;

		// Called on every `CarsFrontEnd::SetScreen` for which a foreign (non-vanilla) screen state is observed.
		auto retrieve_options_list(CarsFrontEnd* _this, CarsFrontEndScreen screen) -> std::optional<std::string>;
	};

	namespace ingame {
		// Installs the necessary hooks for functionality in-game.
		auto install_hooks() -> void;
	};

	// Asks the network to pull a lobby list. Should be called once; continously call `received_lobby_list` after this to check if the lobby list is ready to use.
	inline auto request_lobby_list() -> void {
		SteamMatchmaking()->RequestLobbyList();
		CONTEXT->lobbyListRequested = true;
	}

	// Checks if we have received the lobby list from the network yet. Call this continously to be notified at precisely the right moment.
	inline auto received_lobby_list() -> bool {
		return CONTEXT->lobbyListRequested && CONTEXT->lobbyListReceived;
	}

	// Consumes the lobby list from the global context.
	inline auto consume_lobby_list() -> std::vector<CSteamID> {
		std::vector<CSteamID> list = CONTEXT->lobbyList;
		CONTEXT->lobbyList.clear();
		CONTEXT->lobbyListRequested = false;
		CONTEXT->lobbyListReceived = false;
		return list;
	}

	// Asks the network to join a lobby.
	inline auto request_join(CSteamID lobby) -> void {
		SteamMatchmaking()->JoinLobby(lobby);
		CONTEXT->lobbyJoinRequested = true;
	}

	// Checks if we've joined the lobby that we asked to earlier.
	inline auto has_joined_lobby() -> bool {
		return CONTEXT->lobbyJoinRequested && CONTEXT->lobbyJoinRequestFulfilled;
	}

	// Enables/Disables Axel.
	inline auto set_online(bool active) -> void {
		CONTEXT->isOnline = active;
	}

	// Helper function to check if Axel is active.
	inline auto online() -> bool {
		return CONTEXT->isOnline;
	}

	// Helper function that grabs the player count.
	inline auto player_count() -> int {
		return CONTEXT->playerCount;
	}

	// Helper function that translates a Player ID into an Axel ID. Returns -1 if the given Player ID is not assigned to any lobby members.
	inline auto to_axel_id(int playerId) -> int {
		for (auto i = 0; i < CONTEXT->playerCount; i++) {
			if (CONTEXT->lobbyMembers[i].playerId == playerId) {
				return i;
			}
		}
		return -1;
	}
};