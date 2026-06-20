#pragma once
#include "../Input/SystemInputDriver.hpp"
#include "../Genie/String.hpp"

#include "CActorComponent.hpp"

inline auto CarsVehicle_BumpBashHandle = (void(_cdecl*)(CActor*, CActor*, bool))(0x006be990);
inline auto CarsVehicle_CancelInTheZone = (void(__thiscall*)(class CarsVehicle*, bool))(0x006b32d0);
inline auto CarsVehicle_GoInTheZone = (bool(__thiscall*)(class CarsVehicle*, bool, bool))(0x006b3390);
inline auto CarsVehicle_SetPowerTrainOverride = (void(__thiscall*)(class CarsVehicle*, bool, float, float, float, bool, bool))(0x006c35e0);
inline auto CarEnergy_AddEnergy = (void(__thiscall*)(class CarEnergy*, float, bool, bool, int, const char*, int, bool))(0x00690590);
inline auto CarsControlMapper_SetVibrationDuration = (void(__thiscall*)(class CarsControlMapper*, std::uint32_t, std::uint32_t, int))(0x006646f0);
inline auto CarsControlMapper_SwitchControlScheme = (void(__thiscall*)(class CarsControlMapper*, const Genie::String*))(0x00661230);

class CarsControlMapper : public CActorComponent {
public:
	int m_playerId;
	ControllerInputDriver* m_driver;
	bool m_controlEnabled;
public:
	inline void SetVibrationDuration(std::uint32_t duration_ms, std::uint32_t intensity, int pan = 0) {
		CarsControlMapper_SetVibrationDuration(this, duration_ms, intensity, pan);
	}
	inline void SwitchControlScheme(const Genie::String& scheme) {
		CarsControlMapper_SwitchControlScheme(this, &scheme);
	}
};

class CarEnergy {
public:
	float m_energy;
public:
	inline void AddEnergy(float energy, bool one_shot, bool play_burst, int cho, const char* anchor, int control_index, bool force_my_cho) {
		CarEnergy_AddEnergy(this, energy, one_shot, play_burst, cho, anchor, control_index, force_my_cho);
	}
};
class CBulletVehicle : public CActorComponent {
};
// static_assert(sizeof(CBulletVehicle) == 0x890);

class CarsVehicle : public CBulletVehicle {
public:
	static void BumpBashHandle(CActor* actor, CActor* victim, bool unk) {
		CarsVehicle_BumpBashHandle(actor, victim, unk);
	}
	inline CarEnergy& GetCarEnergy() {
		return *reinterpret_cast<CarEnergy*>(reinterpret_cast<std::uintptr_t>(this) + 0x11F0);
	}
	inline bool IsAIControlled() {
		return *reinterpret_cast<bool*>(reinterpret_cast<std::uintptr_t>(this) + 0x89C);
	}
	inline bool GetInTheZone() {
		return *reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(this) + 0xE4C) != 0;
	}
	inline class ActiveMoves* GetActiveMoves() {
		return *reinterpret_cast<class ActiveMoves**>(reinterpret_cast<std::uintptr_t>(this) + 0xDA4);
	}
	inline void CancelInTheZone(bool cancel) {
		CarsVehicle_CancelInTheZone(this, cancel);
	}
	inline void SetPowerTrainOverride(bool power_train_override, float duration, float param_1, float param_2, bool hide_car = false, bool override = false) {
		CarsVehicle_SetPowerTrainOverride(this, power_train_override, duration, param_1, param_2, hide_car, override);
	}
	inline bool GoInTheZone(bool unk, bool unk2) {
		return CarsVehicle_GoInTheZone(this, unk, unk2);
	}
};
// static_assert(sizeof(CarsVehicle) == 0x1800);