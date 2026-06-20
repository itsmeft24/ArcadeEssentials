#include <span>
#include <mutex>
#include <array>
#include <steam_api.h>
#include "SteamNet.hpp"
#include "../pentane.hpp"

axel::SteamNet::SteamNet() : pollGroup(k_HSteamNetPollGroup_Invalid), m_CallbackOnConnectionStatusChanged(this, &SteamNet::OnConnectionStatusChanged) {
    if (SteamNetworkingSockets() != nullptr) {
        SteamNetworkingSockets()->InitAuthentication();
    }
    if (SteamNetworkingUtils() != nullptr) {
        SteamNetworkingUtils()->InitRelayNetworkAccess();
    }
    if (SteamNetworkingSockets() != nullptr) {
        pollGroup = SteamNetworkingSockets()->CreatePollGroup();
        listenSocket = SteamNetworkingSockets()->CreateListenSocketP2P(0, 0, nullptr);
    }
}

axel::SteamNet::~SteamNet() {
    if (SteamNetworkingSockets() != nullptr) {
        SteamNetworkingSockets()->DestroyPollGroup(pollGroup);
        SteamNetworkingSockets()->CloseListenSocket(listenSocket);
    }
}

void axel::SteamNet::connect(CSteamID peer) {
    if (connections.find(peer) != connections.end()) {
        return;
    }

    SteamNetworkingIdentity id{};
    id.SetSteamID(peer);

    if (SteamUser()->GetSteamID() < peer) {
        HSteamNetConnection hConn = SteamNetworkingSockets()->ConnectP2P(id, 0, 0, nullptr);
        SteamNetworkingSockets()->SetConnectionPollGroup(hConn, pollGroup);
        connections[peer] = hConn;
    }
}

void axel::SteamNet::send_routed_message_async(CSteamID recipient, const std::vector<std::uint8_t>& payload) {
    outbound.push(std::make_pair(recipient, payload));
}

std::chrono::milliseconds axel::SteamNet::estimate_ping(CSteamID peer) {
    auto it = connections.find(peer);
    if (it == connections.end()) {
        logger::log_format("[axel::SteamNet::estimate_ping_ms] Attempted to get ping for unknown peer {}!", peer.ConvertToUint64());
        return std::chrono::milliseconds(0);
    }

    // We pass 0 and nullptr for the lane-specific arguments as we only want general connection stats.
    SteamNetConnectionRealTimeStatus_t status{};
    if (SteamNetworkingSockets()->GetConnectionRealTimeStatus(it->second, &status, 0, nullptr) != k_EResultOK) {
        logger::log_format("[axel::SteamNet::estimate_ping_ms] Error: Failed to retrieve connection status for peer {}!", peer.ConvertToUint64());
        return std::chrono::milliseconds(0);
    }

    return std::chrono::milliseconds(status.m_nPing);
}

std::vector<std::span<std::uint8_t>> axel::SteamNet::get_incoming() {
    std::vector<std::span<std::uint8_t>> result{};
    result.reserve(incoming_message_count);
    for (auto i = 0; i < incoming_message_count; i++) {
        std::span<std::uint8_t> packet(reinterpret_cast<std::uint8_t*>(inbound[i]->m_pData), inbound[i]->m_cbSize);
        result.push_back(packet);
    }
    return result;
}

void axel::SteamNet::send_receive() {
    // Process all outbound mesages and send them out.
    std::pair<CSteamID, std::vector<std::uint8_t>> msg;
    while (outbound.pop(msg)) {
        CSteamID targetSteamID = msg.first;

        auto it = connections.find(targetSteamID);
        if (it == connections.end() || !targetSteamID.IsValid()) {
            continue;
        }

        // logger::log_format("[axel::SteamNet::send_receive] Sending message bound for: {}...", targetSteamID.ConvertToUint64());
        EResult result = SteamNetworkingSockets()->SendMessageToConnection(
            it->second,
            msg.second.data(),
            msg.second.size(),
            k_nSteamNetworkingSend_UnreliableNoDelay,
            nullptr
        );
    }

    // Process all incoming messages by deferring to the global message handler.
    std::array<SteamNetworkingMessage_t*, 32> msgs{};
    incoming_message_count = SteamNetworkingSockets()->ReceiveMessagesOnPollGroup(pollGroup, inbound.data(), inbound.size());
}

void axel::SteamNet::release_received() {
    for (auto i = 0; i < incoming_message_count; i++) {
        inbound[i]->Release();
    }
}

void axel::SteamNet::OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* p) {

    if (p->m_info.m_eState == k_ESteamNetworkingConnectionState_Connecting) {
        SteamNetworkingSockets()->AcceptConnection(p->m_hConn);
        SteamNetworkingSockets()->SetConnectionPollGroup(p->m_hConn, pollGroup);
        connections[p->m_info.m_identityRemote.GetSteamID()] = p->m_hConn;
    }

    if (p->m_info.m_eState == k_ESteamNetworkingConnectionState_ClosedByPeer || p->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally) {
        SteamNetworkingSockets()->CloseConnection(p->m_hConn, 0, nullptr, false);
        if (connections.count(p->m_info.m_identityRemote.GetSteamID()) != 0) {
            connections.erase(p->m_info.m_identityRemote.GetSteamID());
        }
    }
}
