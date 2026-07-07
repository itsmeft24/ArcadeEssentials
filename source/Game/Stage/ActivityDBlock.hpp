#pragma once
#include "DBlock.hpp"
#include "../../Game/DynamicArray.hpp"

class ActivityDBlock : public DBlock {
public:
	float d_maxUpdateSec;
	unsigned int d_involvement;
	float d_fLifetime;
	float d_shutdownDistance;
	bool d_markedForDeath;
	bool d_sendingDeathNotifications;
	bool d_delayDeath;
	bool d_bAllowDeath;
	DynamicArray<ActorHandle> d_deathNotifications;
	DynamicArray<void*> d_systemDeathNotifications;
};