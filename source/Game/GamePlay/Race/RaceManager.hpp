#pragma once
#include "../../Stage/StageEntity.hpp"
#include "../../Utils/Vector3.hpp"
#include "../../Utils/Matrix.hpp"

inline auto	RaceManager_RacerIsAIControlled = (bool(__thiscall*)(class RaceManager*, ActorHandle, int))(0x004f6aa0);
inline auto	RaceManager_GetRaceTime = (float(__thiscall*)(class RaceManager*, int, bool))(0x004f52e0);
inline auto	RaceManager_UpdateRacerControlContexts = (void(__thiscall*)(class RaceManager*))(0x004f5d60);

class RaceManager {
public:


#ifdef WIN32_WII
	enum class RaceState {
		Inactive = 0,
		WaitingToStart = 1,
		Racing = 2,
		ShowTrophy = 3,
		Complete = 4,
		RestartCountdown = 5,
		SpyMission = 6,
	};
#else
	// In Arcade, `RaceState::NetWaiting` is a 'faux state' added by Raw Thrills.
	// By 'faux state', I mean that `RaceManager::GetRaceState` will never return it,
	// and instead will return `RaceState::WaitingToStart`; because of this, very few parts
	// of the game recognize it as a real state. It is basically only used within `RaceManager`.
	enum class RaceState {
		Inactive = 0,
		NetWaiting = 1,
		WaitingToStart = 2,
		Racing = 3,
		ShowTrophy = 4,
		Complete = 5,
		RestartCountdown = 6,
		SpyMission = 7,
	};
#endif

	struct RacerData {
		ActorHandle m_handle;
		Vector3 m_carFwd;
		Vector3 m_carRight;
		Vector3 m_carLeft;
		Vector3 m_position;
		int m_raceRank;
		float m_raceProgress;
		float m_lapPosition;
		Matrix4x4 m_raceStartPosition;
		float m_challengeTimer;
		float m_challengeReplay;
		ActorHandle m_engagedWith;
		bool m_onMyLeft;
		bool m_aiControlled;
		bool m_netControlled;
		void* m_unk;
		void* m_unk2;
		void* m_lazyMouth;
		void* m_eyeComponent;
		void* m_lazyTow;
		void* m_unk3;
		void* m_unk4;
		void* m_unk5;
	};
public:

public:
	inline bool RacerIsAIControlled(ActorHandle act, int indexHint) {
		return RaceManager_RacerIsAIControlled(this, act, indexHint);
	}

	inline int GetRacerCount() {
		return *reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(this) + 0x24);
	}

	inline float GetRaceTime(int team, bool forDisplay = false) {
		return RaceManager_GetRaceTime(this, team, forDisplay);
	}

	inline RaceState GetRaceState() {
		RaceState state = *reinterpret_cast<RaceState*>(reinterpret_cast<std::uintptr_t>(this) + 0x30);
		if (state == RaceState::NetWaiting) {
			return RaceState::WaitingToStart;
		}
		return state;
	}

	inline void UpdateRacerControlContexts() {
		RaceManager_UpdateRacerControlContexts(this);
	}

	inline RacerData* GetRacerData(int i) {
		return *reinterpret_cast<RacerData**>(reinterpret_cast<std::uintptr_t>(this) + 0x20) + i;
	}

	inline int GetPlayerNumber(CActor& actor) {
		Cars2VehicleDBlock* dblock = Cars2VehicleDBlock::Get(actor);
		if (dblock->m_carsControlMapper != nullptr) {
			return dblock->m_carsControlMapper->m_playerId;
		}
		return 0;	
	}
};

static_assert(sizeof(RaceManager::RacerData) == 0xB0);

inline RaceManager** g_RaceManager = reinterpret_cast<RaceManager**>(0x018ae0fc);