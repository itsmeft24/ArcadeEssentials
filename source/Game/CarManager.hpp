#pragma once
#include <cstdint>
#include "CActor.hpp"

inline auto CarManager_GetCarClass = (int(__thiscall*)(class CarManager*, std::uint32_t))(0x004d7550);
inline auto CarManager_GetCarId = (std::uint32_t(__thiscall*)(class CarManager*, ActorHandle))(0x004d6460);
inline auto CarManager_GetCarIdByName = (int(__thiscall*)(class CarManager*, const char*))(0x004d64a0);
inline auto CarManager_GetSplitScreenType = (const char*(__thiscall*)(class CarManager*, std::uint32_t))(0x004dea40);

class CarManager {
public:
	struct CarInfo {
		char* name;
		char* mask;
		unsigned int actorType;
		unsigned int indexInList;
		unsigned int type;
		unsigned int weapons[10];
		unsigned int weaponCount;
		unsigned int carClass;
		unsigned int mass;
		float tireWidthR;
		float tireWidthF;
		float power;
		float steerWBAdjust;
		bool isDLC;
		bool humanPlayable;
		bool isRaceOnly;
		char* metaZipPath;
		char* offerId;
		char* productId;
		char* lockName;
		char* pipCameraGroupName;
		char* splitScreenType;
		unsigned int loadedFromDlcLua;
		char* engine;
		char* gearShift;
		char* dialogName;
		char* dialogCode;
	};

	enum class CarClass : int {
		Light = 0,
		Medium = 1,
		Heavy = 2,
	};
	
	struct ActiveMovesSettings {
		float bunnyHopHeight;
		float sideStepHeight;
		float sideStepWidth;
		float sideStepBashPenaltyAmount;
		float sideStepBashThrowSideAttacker;
		float sideStepBashThrowSideVictim;
		float sideStepBashThrowUpVictim;
		float sideStepBashTurboThrowSideAttacker;
		float sideStepBashTurboThrowSideVictim;
		float sideStepBashTurboThrowUpVictim;
		float sideStepBashThrowSideAttackerClassFactor;
		float sideStepBashThrowSideVictimClassFactor;
		float sideStepBashThrowUpVictimClassFactor;
		float twoWheelingTimeLimit;
		float twoWheelingAirTimeLimit;
		float twoWheelingMinSpeedMPH;
		float twoWheelingSlowSpeedMPH;
		float twoWheelingSteerFactor;
		float twoWheelingOffBalanceThreshold;
		float twoWheelingOffBalanceTimeLimit;
		bool twoWheelingDebugDrawBalance;
		float twoWheelingBalanceRollAmt;
		float twoWheelingBalanceYawAmt;
		float twoWheelingBalanceKp;
		float twoWheelingBalanceKi;
		float twoWheelingBalanceKd;
		float twoWheelingBumpThreshold;
		float jumpTrickMinHeight;
		float turn180SpeedLimit;
		float backwardsDrivingMinSpeedMPH;
		float bunnyHopUpBoost;
		float bunnyHopUpBoostStartTime;
		float bunnyHopUpBoostDuration;
		float bunnyHopDownBoost;
		float bunnyHopDownBoostStartTime;
		float bunnyHopDownBoostDuration;
		float jumpTrickFailLRSpeedDrop;
		float jumpTrickFailLRCrashTime;
		float jumpTrickFailUDCrashTime;
	};
	
	struct SteeringSettings {};
	
	struct GameSettings {};

	int mcqueenIndex;
	CarInfo* carInfos;
	unsigned int carInfosLen;
public:
	std::uint32_t GetCarId(ActorHandle handle) {
		return CarManager_GetCarId(this, handle);
	}
	int GetCarId(const char* name) {
		return CarManager_GetCarIdByName(this, name);
	}
	const char* GetSplitScreenType(std::uint32_t carId) {
		return CarManager_GetSplitScreenType(this, carId);
	}
	CarClass GetCarClass(std::uint32_t id) {
		return static_cast<CarClass>(CarManager_GetCarClass(this, id));
	}
};

inline CarManager** g_CarManager = reinterpret_cast<CarManager**>(0x018ae0e8);
