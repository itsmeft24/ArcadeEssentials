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

axel::Context::Context() : m_CallbackOnLobbyEnter(this, &Context::OnLobbyEnter), m_CallbackOnLobbyChatUpdate(this, &Context::OnLobbyChatUpdate), m_CallbackOnLobbyCreate(this, &Context::OnLobbyCreate), m_CallbackOnRecieveLobbyList(this, &Context::OnRecieveLobbyList), m_CallbackOnLobbyDataUpdate(this, &Context::OnLobbyDataUpdate) {
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
        if (std::string_view(SteamMatchmaking()->GetLobbyData(lobby, "is_axel")) == "true") {
            lobbyList.push_back(lobby);
            logger::log_format("[axel::Context::RecieveLobbyList] Found Lobby: {} with {} players.", lobby.ConvertToUint64(), SteamMatchmaking()->GetNumLobbyMembers(lobby));
        }
    }
    if (lobbyListRequested) {
        lobbyListReceived = true;
    }
}

void axel::Context::OnLobbyCreate(LobbyCreated_t* e) {
    switch (e->m_eResult) {
    case k_EResultOK:
        if (lobbyCreateRequested) {
            lobbyCreated = true;
        }
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
    }

    // Now, we need to make sure that the we are Player ID 0 no matter what, even if our Axel ID is not 0.
    lobbyMembers[myAxelId].playerId = 0;
}

// Called when someone *else* joins or leaves our lobby.
void axel::Context::OnLobbyChatUpdate(LobbyChatUpdate_t* e) {
    logger::log_format("[axel::Context::OnLobbyChatUpdate] Chat Updated!");

    uint32 stateChange = e->m_rgfChatMemberStateChange;
    CSteamID changedUser = e->m_ulSteamIDUserChanged;

    if (stateChange & k_EChatMemberStateChangeEntered) {
        logger::log_format("[axel::Context::OnLobbyChatUpdate] Joinee: {}, Lobby Size: {}", changedUser.ConvertToUint64(), SteamMatchmaking()->GetNumLobbyMembers(lobbyID));

        int axelId = -1;
        const char* axelIdStr = SteamMatchmaking()->GetLobbyMemberData(lobbyID, changedUser, "axel_id");
        if (axelIdStr != nullptr && strlen(axelIdStr) != 0) {
            axelId = std::atoi(axelIdStr);
        }

        if (axelId == -1) {
            axelId = SteamMatchmaking()->GetNumLobbyMembers(lobbyID) - 1;
            logger::log_format("[axel::Context::OnLobbyChatUpdate] Remote user did not assign themselves an Axel ID. Defaulting to: {}...", axelId);
        }

        lobbyMembers[axelId].steamId = changedUser;
        playerCount++;
        playerCountChanged = true;
    }
    else if (stateChange & (k_EChatMemberStateChangeLeft | k_EChatMemberStateChangeDisconnected | k_EChatMemberStateChangeKicked | k_EChatMemberStateChangeBanned)) {
        logger::log_format("[axel::Context::OnLobbyChatUpdate] User left: {}, Lobby Size: {}", changedUser.ConvertToUint64(), SteamMatchmaking()->GetNumLobbyMembers(lobbyID));

        // 1. Find the Axel ID of the person who left
        int leftAxelId = -1;
        for (int i = 0; i < playerCount; i++) {
            if (lobbyMembers[i].steamId == changedUser) {
                leftAxelId = i;
                break;
            }
        }

        if (leftAxelId == -1) {
            logger::log_format("[axel::Context::OnLobbyChatUpdate] Warning: Could not find leaving user in local state!");
            return;
        }

        // 2. Shift entries leftward to fill the gap
        // (We must shift all state arrays indexed by Axel ID)
        for (int i = leftAxelId; i < playerCount - 1; i++) {
            lobbyMembers[i] = lobbyMembers[i + 1];
            vehicleStates[i] = vehicleStates[i + 1];
            isMemberReady[i] = isMemberReady[i + 1];
        }

        // Clear the now-duplicate last entry
        int lastIndex = playerCount - 1;
        lobbyMembers[lastIndex] = LobbyMemberState(); // Reset CSteamID to 0ull and playerId to -1
        vehicleStates[lastIndex] = VehicleState();
        isMemberReady[lastIndex] = false;

        // 3. Update our own Axel ID if we were situated after the player who left
        if (myAxelId > leftAxelId) {
            myAxelId--;
            // Broadcast our shifted Axel ID so new joiners map us correctly
            SteamMatchmaking()->SetLobbyMemberData(lobbyID, "axel_id", std::to_string(myAxelId).c_str());
        }

        // 4. Check for host migration (if the owner left, Steam assigns a new one automatically)
        isHost = SteamMatchmaking()->GetLobbyOwner(lobbyID) == SteamUser()->GetSteamID();
        if (isHost) {
            logger::log_format("[axel::Context::OnLobbyChatUpdate] We are now the lobby host.");
        }

        playerCount--;
        playerCountChanged = true;
    }

    // --- RECALCULATE PLAYER IDs ---
    // Whether someone joins or leaves, it's safest to recalculate playerId for everyone
    // to ensure '0' is always local, and everyone else counts up consecutively.
    int currentOtherPlayerId = 1;
    for (int i = 0; i < playerCount; i++) {
        if (i == myAxelId) {
            lobbyMembers[i].playerId = 0; // We are always Player 0 locally
        }
        else {
            lobbyMembers[i].playerId = currentOtherPlayerId++;
        }
    }
}

void axel::Context::OnLobbyDataUpdate(LobbyDataUpdate_t* e) {
    // Metadata for the actual lobby was changed...
    if (e->m_ulSteamIDLobby == e->m_ulSteamIDMember) {
        logger::log_format("[axel::Context::OnLobbyDataUpdate] Lobby metadata was changed.");
    }
    // Metadata for a particular lobby member was changed...
    else {
        logger::log_format("[axel::Context::OnLobbyDataUpdate] Lobby data for member: {} was changed.", e->m_ulSteamIDMember);
        selectedCarChanged = true;
    }
}