#pragma once
#include "CActorComponent.hpp"

class WeaponInventory : public CActorComponent {
public:
	void* m_list;
	int m_size;
	int m_maxSize;
	ActorHandle m_alternateOwner;
	int m_current;
public:
};

class CarsWeaponInventory : public WeaponInventory {
};