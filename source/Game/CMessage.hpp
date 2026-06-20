#pragma once
#include "Stage/StageEntity.hpp"

inline auto CMessageDispatcher_SendMessageToAll = (void(__thiscall*)(class CMessageDispatcher*, const char*, void*, ActorHandle))(0x00cef410);
inline auto CMessageDispatcher_SendMessageToActor = (bool(__thiscall*)(class CMessageDispatcher*, const char*, void*, ActorHandle, ActorHandle))(0x00cef520);

class CMessageDispatcher {
public:
	void SendMessageToAll(const char* name, void* data, ActorHandle sender) {
		CMessageDispatcher_SendMessageToAll(this, name, data, sender);
	}
	void SendMessageToActor(const char* name, void* data, ActorHandle sender, ActorHandle actor) {
		CMessageDispatcher_SendMessageToActor(this, name, data, sender, actor);
	}
};
class CMessagePair;

class CMessageOwner {
private:
	CMessagePair* m_pairHead;
	bool m_isMaster;
	unsigned int m_sessionID;
};

class CMessageHandler {
private:
	CMessagePair* m_pair;
	CMessageHandler* m_next;
	unsigned int m_rules;
	bool m_deleteWhenDefunct;
public:
	virtual ~CMessageHandler();
	virtual void Terminate();
	virtual void HandleMessage(void* data, ActorHandle sender, bool posted);
	virtual void HandleMessage(void* data, ActorHandle sender, bool posted, const char* messageName);
};

template <typename T> struct GenericMessageHandler : public CMessageHandler {
public:
	T* m_reference;
};

inline CMessageDispatcher** g_MessageDispatcher = reinterpret_cast<CMessageDispatcher**>(0x01926ef0);