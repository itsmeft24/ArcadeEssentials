#pragma once
#include <array>
#include <vector>
#include <atomic>
#include <steam_api.h>
#include <memory>
#include "SteamNet.hpp"
#include "AxelVehicleState.hpp"

namespace axel {

	struct LobbyMemberState {
		CSteamID steamId = 0ull;
		int playerId = -1;
	};

	class Context {
	private:
		STEAM_CALLBACK(Context, OnRecieveLobbyList, LobbyMatchList_t, m_CallbackOnRecieveLobbyList);
		STEAM_CALLBACK(Context, OnLobbyCreate, LobbyCreated_t, m_CallbackOnLobbyCreate);
		STEAM_CALLBACK(Context, OnLobbyEnter, LobbyEnter_t, m_CallbackOnLobbyEnter);
		STEAM_CALLBACK(Context, OnLobbyChatUpdate, LobbyChatUpdate_t, m_CallbackOnLobbyChatUpdate);
	public:
		// The following members are 'State Variables', meaning they're pretty much always accessible and
		// reflect the current session/lobby state:

		// Whether or not Axel is active.
		bool isOnline = false;
		// The local player's Axel ID.
		int myAxelId = -1;
		// Whether or not the local player is currently in a lobby.
		bool inLobby = false;
		// Whether or not the local player is the party leader.
		bool isHost = false;
		// The number of people in the current lobby.
		int playerCount = 1;
		// The Axel IDs, Player IDs, and Steam IDs of each lobby member. This array is indexed by Axel ID.
		std::array<LobbyMemberState, 10> lobbyMembers = {};
		// The Steam ID of the current lobby, if we're in one.
		CSteamID lobbyID;
		// The current, unfiltered `VehicleState` of each lobby member.
		std::array<VehicleState, 10> vehicleStates = {};
		VehicleState myPreviousState = {};

		// These are used explicitly for facilitating or responding to state *changes*, and often represent
		// 'events' that are fired once a specific packet is received:

		// Whether or not the player count changed since the last call to `SteamAPI_RunCallbacks`.
		bool playerCountChanged = false;
		// Whether or not a lobby list was requested.
		bool lobbyListRequested = false;
		// Whether or not a lobby list has been recieved.
		bool lobbyListReceived = false;
		// The lobby list we just recieved.
		std::vector<CSteamID> lobbyList = {};
		// Whether or not a lobby join request has been fired.
		bool lobbyJoinRequested = false;
		// Whether or not the lobby join request has been fulfilled.
		bool lobbyJoinRequestFulfilled = false;

		// Whether or not each lobby member is ready to start the race.
		std::array<bool, 10> isMemberReady = {};
		// Whether or not all opponents are ready to start the race. This is only read/written to if we are the party leader.
		bool allOpponentsReady = false;
		// When we should exit the loading screen.
		std::chrono::utc_clock::time_point startTime = {};
		// Whether or not the party leader has told us to start the race. This is only read/written to if we are NOT the party leader.
		bool hostRequestedGameStart = false;

		// The network instance.
		std::unique_ptr<SteamNet> network = nullptr;
	public:
		Context();
		auto update_network() -> void;
	};

	extern std::unique_ptr<Context> CONTEXT;

	auto init_context() -> void;
	auto update_context() -> void;
};