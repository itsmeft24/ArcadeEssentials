#include "AxelContext.hpp"
#include "../pentane.hpp"
#include "AxelMessage.hpp"

std::unique_ptr<axel::Context> axel::CONTEXT = nullptr;

auto axel::init_context() -> void {
    if (!SteamAPI_Init()) {
        logger::log_format("[axel::init_context] Failed to initialize Steam API.");
    }
    CONTEXT = std::make_unique<Context>();
}

auto axel::update_context() -> void {
    CONTEXT->update_network();
    SteamAPI_RunCallbacks();
}

axel::Context::Context() : m_CallbackOnLobbyEnter(this, &Context::OnLobbyEnter), m_CallbackOnLobbyChatUpdate(this, &Context::OnLobbyChatUpdate), m_CallbackOnLobbyCreate(this, &Context::OnLobbyCreate), m_CallbackOnRecieveLobbyList(this, &Context::OnRecieveLobbyList) {
    network = std::make_unique<SteamNet>();
}

auto axel::Context::update_network() -> void {
    
    // Send and recieve all outbound/inbound messages.
    network->send_receive();
    const auto& incoming = network->get_incoming();
    for (const auto& message : incoming) {
        axel::message::handle_incoming(message);
    }
    network->release_received();

    // If we're in a lobby, we need to add connections to every lobby member.
    if (inLobby) {
        for (auto i = 0; i < SteamMatchmaking()->GetNumLobbyMembers(lobbyID); i++) {
            CSteamID other = SteamMatchmaking()->GetLobbyMemberByIndex(lobbyID, i);
            if (other != SteamUser()->GetSteamID()) {
                network->connect(other);
            }
        }
    }
}

void axel::Context::OnRecieveLobbyList(LobbyMatchList_t* pLobbyMatchList) {
    logger::log_format("[axel::Context::RecieveLobbyList] Recieved lobby list!");
    lobbyList.clear();
    for (std::size_t i = 0; i < pLobbyMatchList->m_nLobbiesMatching; i++) {
        CSteamID lobby = SteamMatchmaking()->GetLobbyByIndex(i);
        lobbyList.push_back(lobby);
        logger::log_format("[axel::Context::RecieveLobbyList] Found Lobby: {} with {} players.", lobby.ConvertToUint64(), SteamMatchmaking()->GetNumLobbyMembers(lobby));
    }
    if (lobbyListRequested) {
        lobbyListReceived = true;
    }
}

void axel::Context::OnLobbyCreate(LobbyCreated_t* e) {
    switch (e->m_eResult) {
    case k_EResultOK:
        logger::log_format("[axel::Context::OnLobbyCreate] Successfully created lobby: {}!", e->m_ulSteamIDLobby);
        break;
    case k_EResultNoConnection:
        logger::log_format("[axel::Context::OnLobbyCreate] Failed to create lobby! No connection to Steamworks backend.");
        break;
    case k_EResultTimeout:
        logger::log_format("[axel::Context::OnLobbyCreate] Failed to create lobby! Server response timed out.");
        break;
    case k_EResultFail:
        logger::log_format("[axel::Context::OnLobbyCreate] Failed to create lobby! Server responded with an internal error.");
        break;
    case k_EResultAccessDenied:
        logger::log_format("[axel::Context::OnLobbyCreate] Failed to create lobby! Access was denied.");
        break;
    case k_EResultLimitExceeded:
        logger::log_format("[axel::Context::OnLobbyCreate] Failed to create lobby! Too many lobbies were created from this client.");
        break;
    }
}

// Called when we join/create a lobby.
// It is important to note that party leaders *also* receive this event.
void axel::Context::OnLobbyEnter(LobbyEnter_t* e) {
    logger::log_format("[axel::Context::OnLobbyEnter] Joining lobby: {}...", e->m_ulSteamIDLobby);

    if (lobbyJoinRequested) {
        lobbyJoinRequestFulfilled = true;
    }

    lobbyID = e->m_ulSteamIDLobby;
    isHost = SteamMatchmaking()->GetLobbyOwner(lobbyID) == SteamUser()->GetSteamID();
    inLobby = true;

    // This `count` DOES include us, so we set the session context `playerCount` to exactly the same as `count`.
    int count = SteamMatchmaking()->GetNumLobbyMembers(lobbyID);

    playerCount = count;
    myAxelId = count - 1;
    lobbyMembers[count - 1].steamId = SteamUser()->GetSteamID();
    playerCountChanged = true;

    // Let's announce our Axel ID.
    SteamMatchmaking()->SetLobbyMemberData(lobbyID, "axel_id", std::to_string(myAxelId).c_str());

    for (int i = 0; i < count - 1; i++) {
        CSteamID member = SteamMatchmaking()->GetLobbyMemberByIndex(lobbyID, i);
        const char* axelIdStr = SteamMatchmaking()->GetLobbyMemberData(lobbyID, member, "axel_id");
        if (axelIdStr == nullptr) {
            continue;
        }
        if (strlen(axelIdStr) == 0) {
            continue;
        }
        int axelId = std::atoi(axelIdStr);
        if (axelId >= 0 && axelId < 10) {
            lobbyMembers[axelId].steamId = member;
        }
    }

    if (myAxelId == -1) {
        logger::log_format("[axel::Context::OnLobbyEnter] We weren't assigned an Axel ID!");
    }

    // Here, we iterate over all Axel IDs before ourselves, and assign their Player IDs such that Axel ID 0 gets assigned Player ID 1, Axel ID 1 gets assigned Player 2, etc.
    for (std::size_t i = 0; i < myAxelId; i++) {
        lobbyMembers[i].playerId = i + 1;
        // axel::GAME_SESSION.axelIdToPlayerId[i] = i + 1;
        // axel::GAME_SESSION.playerIdToAxelId[i + 1] = i;
    }

    // Now, we need to make sure that the we are Player ID 0 no matter what, even if our Axel ID is not 0.
    lobbyMembers[myAxelId].playerId = 0;
    // axel::GAME_SESSION.axelIdToPlayerId[axel::GAME_SESSION.myAxelId.get()] = 0;
    // axel::GAME_SESSION.playerIdToAxelId[0] = axel::GAME_SESSION.myAxelId.get();

    // Finally, we set up `playerIdToSteamId` based off of `axelIdToPlayerId` and `axelIdToSteamId`.
    /*
    for (std::size_t i = 0; i < axel::GAME_SESSION.playerCount; i++) {
        axel::GAME_SESSION.playerIdToSteamId[axel::GAME_SESSION.axelIdToPlayerId[i]] = axel::GAME_SESSION.axelIdToSteamId[i];
    }
    */
}

// Called when someone *else* joins or leaves our lobby.
void axel::Context::OnLobbyChatUpdate(LobbyChatUpdate_t* e) {
    logger::log_format("[axel::Context::OnLobbyChatUpdate] Chat Updated!");

    bool hasJoined = (e->m_rgfChatMemberStateChange & k_EChatMemberStateChangeEntered) != 0;
    if (!hasJoined) {
        return;
    }
    /*
    When someone leaves the lobby, we need to get our Axel ID as well as the person who left, and carefully
    shift everyone's entries in the session context.
    */

    CSteamID joined = e->m_ulSteamIDUserChanged;

    logger::log_format("[axel::Context::OnLobbyChatUpdate] Joinee: {}, Lobby Size: {}", e->m_ulSteamIDUserChanged, SteamMatchmaking()->GetNumLobbyMembers(lobbyID));

    int axelId = -1;
    const char* axelIdStr = SteamMatchmaking()->GetLobbyMemberData(lobbyID, joined, "axel_id");
    if (axelIdStr != nullptr) {
        if (strlen(axelIdStr) != 0) {
            axelId = std::atoi(axelIdStr);
        }
    }

    if (axelId == -1) {
        axelId = SteamMatchmaking()->GetNumLobbyMembers(lobbyID) - 1;
        logger::log_format("[axel::Context::OnLobbyChatUpdate] Remote user did not assign themselves an Axel ID. Defaulting to: {}...", axelId);
    }

    lobbyMembers[axelId].playerId = playerCount;
    lobbyMembers[axelId].steamId = joined;
    playerCount++;
    playerCountChanged = true;

    // GAME_SESSION.axelIdToPlayerId[axelId] = GAME_SESSION.playerCount;
    // GAME_SESSION.playerIdToAxelId[GAME_SESSION.playerCount] = axelId;
    // GAME_SESSION.axelIdToSteamId[axelId] = joined;
    // GAME_SESSION.playerIdToSteamId[GAME_SESSION.playerCount] = joined;
    // GAME_SESSION.playerCount++;
    // GAME_SESSION.isDirty = true;
}