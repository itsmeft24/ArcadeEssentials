#pragma once
#include <span>

namespace axel::message {
	auto handle_incoming(const std::span<const std::uint8_t> message) -> void;
};