#pragma once
#include "../../Stage/Cars2VehicleDBlock.hpp"
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

	struct ResetLocator {
		Vector3 position;
		float heading;
		float trackPosition;
	};

public:
	bool m_abortRace;
	bool m_racerListLocked;
	float m_arcade180sTimer;
	float m_arcadeAutoPilotTimer;
	bool m_arcadeEngageAutoPilot;
	bool m_arcadeBackwardsDriving;
	float m_arcadeBwdTimer;
	unsigned int m_auroraCategory;
	struct MissionGameInf* m_missionGame;
	Genie::Array<RaceManager::RacerData> m_racerDataList;
	unsigned int m_raceState;
	int m_curLap;
	float m_lapLength;
	float m_raceStartTime;
	int m_restartPlaceRacerFrameCount;
	struct RaceManagerNode* m_meridianNode;
	float m_raceRestartCountDown;
	struct CurveBSpline* m_lapPath;
	bool m_raceEvaluated;
	int m_initFacial;
	int m_facialOn;
	int m_facialOff;
	unsigned int m_flags;
	unsigned int m_humanRacerCount;
	unsigned int m_leadingHumanRacerHandle;
	int m_leadingHumanRacerIndex;
	bool m_leadingHumanRacerValid;
	int m_leadingRacerHandle;
	int m_leadingRacerIndex;
	bool m_leadingRacerValid;
	unsigned int m_lastHumanRacerIndex;
	bool m_ranksValid;
	struct CACChallengeSettings* m_cacChallengeSettings;
	CMessageOwner m_owner;
	GenericMessageHandler<RaceManager> m_forceRaceStart;
	GenericMessageHandler<RaceManager> m_eventStart;
	GenericMessageHandler<RaceManager> m_endTutorial;
	GenericMessageHandler<RaceManager> m_toggleRainFX;
	bool m_rainFXOn;
	unsigned int m_pipIdTauntLeft;
	unsigned int m_pipIdTauntRight;
	bool m_lapPathChecked;
	bool m_unknown0;
	bool m_trackMessageSent;
	bool m_showingResults;
	bool m_forceOverride;
	bool m_facialOverride;
	bool m_unknown1;
	bool m_unknown2;
	bool m_isTeamRace;
	int m_unknown3;
	bool m_arcadeRioDropped;
	bool m_highGravEnabled;
	DynamicArray<ResetLocator> m_locatorList;
	int m_initializeWait;
	unsigned int m_trophyTimer;
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
		return &m_racerDataList[i];
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
static_assert(sizeof(RaceManager) == 0x13c);

inline RaceManager** g_RaceManager = reinterpret_cast<RaceManager**>(0x018ae0fc);