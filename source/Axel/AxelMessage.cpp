#include "AxelMessage.hpp"
#include "AxelMessageTypes.hpp"
#include "AxelContext.hpp"
#include "../pentane.hpp"

auto axel::message::handle_incoming(const std::span<const std::uint8_t> message) -> void {
	auto header = reinterpret_cast<const axel::message::MessageHeader*>(message.data());
	switch (header->type) {
		case axel::message::MessageType::PlayerChangedCharacters:
			break;
		case axel::message::MessageType::ExitFrontEnd:
			break;
		case axel::message::MessageType::VehicleStatePeriodic:
			axel::CONTEXT->vehicleStates[header->sender_axel_id] = reinterpret_cast<const axel::message::VehicleStatePacket*>(header)->state;
			break;
		case axel::message::MessageType::DoneLoading:
			if (axel::CONTEXT->isHost) {
				logger::log_format("[axel::message::handle_incoming] Player: {} has finished loading.", header->sender_axel_id);
				axel::CONTEXT->isMemberReady[header->sender_axel_id] = true;
			}
			break;
		case axel::message::MessageType::StartGame:
			if (!axel::CONTEXT->isHost) {
				std::chrono::milliseconds startInMs = std::chrono::milliseconds(reinterpret_cast<const axel::message::StartGamePacket*>(header)->start_in_ms);
				startInMs -= axel::CONTEXT->network->estimate_ping(axel::CONTEXT->lobbyMembers[header->sender_axel_id].steamId);
				logger::log_format("[axel::message::handle_incoming] Party leader has told us to start the race in: {} ms.", startInMs);
				axel::CONTEXT->hostRequestedGameStart = true;
				axel::CONTEXT->startTime = std::chrono::utc_clock::now() + startInMs;
			}
			break;
	}
}
