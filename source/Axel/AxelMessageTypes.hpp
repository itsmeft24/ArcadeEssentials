#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <string_view>
#include "AxelVehicleState.hpp"

#pragma pack(push, 1)

namespace axel::message {

    enum class MessageType : std::uint8_t {
        PlayerChangedCharacters = 1,
        ExitFrontEnd = 2,

        VehicleStatePeriodic = 3,
        DoneLoading = 4,
        StartGame = 5,
    };

    struct MessageHeader {
    public:
        MessageType type;
        std::uint8_t sender_axel_id;
        std::uint8_t reciever_axel_id;
    };

    struct PlayerChangedCharactersPacket : public MessageHeader {
        char character_name[32]{};
        static inline PlayerChangedCharactersPacket make(std::uint8_t sender, std::uint8_t recipient, std::string_view character) {
            PlayerChangedCharactersPacket ret{
                MessageType::PlayerChangedCharacters,
                sender,
                recipient,
            };
            std::memcpy(ret.character_name, character.data(), (std::min)(character.size(), sizeof(character_name) - 1));
            return ret;
        }
    };

    struct ExitFrontEndPacket : public MessageHeader {
        static inline ExitFrontEndPacket make(std::uint8_t sender, std::uint8_t recipient) {
            ExitFrontEndPacket ret{
                MessageType::ExitFrontEnd,
                sender,
                recipient
            };
            return ret;
        }
    };

    struct VehicleStatePacket : public MessageHeader {
        axel::VehicleState state;
        static inline VehicleStatePacket make(std::uint8_t sender, std::uint8_t recipient, const axel::VehicleState& state) {
            VehicleStatePacket ret{
                MessageType::VehicleStatePeriodic,
                sender,
                recipient,
                state
            };
            return ret;
        }
    };

    struct DoneLoadingPacket : public MessageHeader {
        static inline DoneLoadingPacket make(std::uint8_t sender, std::uint8_t recipient) {
            DoneLoadingPacket ret{
                MessageType::DoneLoading,
                sender,
                recipient
            };
            return ret;
        }
    };

    struct StartGamePacket : public MessageHeader {
        unsigned long long start_in_ms;
        static inline StartGamePacket make(std::uint8_t sender, std::uint8_t recipient, unsigned long long start_in_ms) {
            StartGamePacket ret{
                MessageType::StartGame,
                sender,
                recipient,
                start_in_ms
            };
            return ret;
        }
    };
};

#pragma pack(pop)