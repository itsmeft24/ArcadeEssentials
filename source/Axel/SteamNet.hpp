#pragma once
#include <span>
#include <unordered_map>
#include <chrono>
#include <vector>
#include <isteamnetworkingsockets.h>
#include <isteamnetworkingutils.h>
#include "TSQueue.hpp"

namespace axel {
    struct CSteamIDHash {
        std::size_t operator()(const CSteamID& id) const noexcept {
            return id.ConvertToUint64();
        }
    };

    struct CSteamIDEqual {
        bool operator()(const CSteamID& lhs, const CSteamID& rhs) const noexcept {
            return lhs == rhs;
        }
    };

    class SteamNet {
    private:
        STEAM_CALLBACK(SteamNet, OnConnectionStatusChanged, SteamNetConnectionStatusChangedCallback_t, m_CallbackOnConnectionStatusChanged);
    private:
        std::unordered_map<CSteamID, HSteamNetConnection, CSteamIDHash, CSteamIDEqual> connections;
        HSteamNetPollGroup pollGroup = k_HSteamNetPollGroup_Invalid;
        HSteamListenSocket listenSocket = k_HSteamListenSocket_Invalid;
        axel::util::TSQueue<std::pair<CSteamID, std::vector<std::uint8_t>>> outbound;
        std::array<SteamNetworkingMessage_t*, 32> inbound;
        std::size_t incoming_message_count;
    public:
        SteamNet();
        ~SteamNet();
        void send_receive();
        void release_received();
        void connect(CSteamID peer);
        void send_routed_message_async(CSteamID recipient, const std::vector<std::uint8_t>& payload);
        std::chrono::milliseconds estimate_ping(CSteamID peer);
        std::vector<std::span<std::uint8_t>> get_incoming();
    };
};