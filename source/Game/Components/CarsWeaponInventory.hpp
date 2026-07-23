#pragma once
#include "CActorComponent.hpp"

inline auto CarsWeaponInventory_FireSelectedWeapon = (bool (__thiscall*)(void*, int))(0x005c22b0);

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
public:
	enum class WeaponIndex : int {
		Invalid = -1,
		Leech = 0,
		RCSkate,
		SatBlitz,
		SatQuake,
		MachineGun,
		OilSlick,
		Missile,
		MissileThreeShot,
		ImpactMine,
		XFactor,
		Max,
	};
	inline WeaponIndex GetCurrentWeaponIndex() {
		return *reinterpret_cast<WeaponIndex*>(reinterpret_cast<std::uintptr_t>(this) + 0x17C);
	}
	inline bool FireSelectedWeapon(int mainOrRear) {
		return CarsWeaponInventory_FireSelectedWeapon(this, mainOrRear);
	}
};