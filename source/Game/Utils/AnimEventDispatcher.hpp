#pragma once
#include "../Stage/StageEntity.hpp"

inline auto AnimEventDispatcher_SendEvent = (void(_cdecl*)(const char*, AgentID, void*))(0x00f0f7a0);
inline auto AnimEventDispatcher_SendEventByHash = (void(_cdecl*)(unsigned int, AgentID, void*))(0x00f0f7d0);
inline auto AnimEventDispatcher_GetEventHash = (unsigned int(_cdecl*)(const char*))(0x00f0f940);

class AnimEventDispatcher {
public:
	static inline void SendEvent(const char* event, AgentID target, void* data = nullptr) {
		AnimEventDispatcher_SendEvent(event, target, data);
	}
	static inline void SendEvent(unsigned int eventHash, AgentID target, void* data = nullptr) {
		AnimEventDispatcher_SendEventByHash(eventHash, target, data);
	}
	static inline unsigned int GetEventHash(const char* event) {
		return AnimEventDispatcher_GetEventHash(event);
	}
};