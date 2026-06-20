#pragma once
#include <cstdint>
#include <array>
#include "CMessage.hpp"

inline auto GameProgressionManager_GetAICarCount = (int(__thiscall*)(class GameProgressionManager*))(0x004ead80);
inline auto GameProgressionManager_SetAICarCount = (void(__thiscall*)(class GameProgressionManager*, int))(0x004eadc0);
inline auto GameProgressionManager_SetMissionTimeByMode = (void(__thiscall*)(class GameProgressionManager*, bool))(0x004e8400);
inline auto GameProgressionManager_FUN_004ebab0 = (void(__thiscall*)(class GameProgressionManager*, int))(0x004ebab0);
inline auto GameProgressionManager_FUN_004ebaf0 = (void(__thiscall*)(class GameProgressionManager*, int))(0x004ebaf0);
inline auto GameProgressionManager_FormatStoryMission = (char* (__thiscall*)(class GameProgressionManager*, int, int))(0x004eaf90);

class GameProgressionManager {
public:
	enum class MissionMode {
		Race,
		Bomb,
		Hunter,
		Arena,
		Collect,
		Pursuit, // Unused
		Takedown,
		Tutorial,
	};

    enum class CheatState {
        Uninitialized = -1,
        Inactive = 0,
        Active = 1,
    };

    enum class Cheat {
        StartWithMaxEnergy,
        UnlimitedEnergy,
        StartWithWGP, // Unlock Lazers
        StartWithMachineGuns, // Lock N' Load
        UnlockAllCars,
        UnlockAllTracks,
        UnlockAllModes,
        Max,
    };
	
    struct MissionModeInfo {
		const char* name;
		const char* localized_name;
		MissionMode mode;
	};

	struct AllowedWeaponList {
		unsigned int count;
		char* allowedWeapons[6];
		bool weaponsDefaulted;
		char* weaponSetName;
	};

    struct TakedownSettings {
        unsigned int m_takeDownKillCountGold;
        unsigned int m_takeDownKillCountSilver;
        unsigned int m_takeDownKillCountBronze;
        unsigned char field43_0xa8;
        unsigned char field44_0xa9;
        unsigned char field45_0xaa;
        unsigned char field46_0xab;
        unsigned char field47_0xac;
        unsigned char field48_0xad;
        unsigned char field49_0xae;
        unsigned char field50_0xaf;
        unsigned char field51_0xb0;
        unsigned char field52_0xb1;
        unsigned char field53_0xb2;
        unsigned char field54_0xb3;
        unsigned char field55_0xb4;
        unsigned char field56_0xb5;
        unsigned char field57_0xb6;
        unsigned char field58_0xb7;
        unsigned char field59_0xb8;
        unsigned char field60_0xb9;
        unsigned char field61_0xba;
        unsigned char field62_0xbb;
        unsigned char field63_0xbc;
        unsigned char field64_0xbd;
        unsigned char field65_0xbe;
        unsigned char field66_0xbf;
    };

    struct CarAIOverrideParameters {
        float aiDrivingSkill;
        float aiRouteFindingSkill;
        float aiTurboSkill;
        float aiWeaponSkill;
        float unknown;
        unsigned int unknown2;
        bool unknown3;
        bool hasCharacterDuplicate;
        char unknown4[0x27];
    };

    struct MissionSettings {
        unsigned int unused0;
        int m_challengeIndex;
        unsigned int m_missionId;
        bool m_isStoryMission;
        char* m_fileName;
        char* m_title;
        char* m_description;
        char* m_image;
        char* m_length;
        char* m_fullTrackName;
        char* m_locationName;
        int m_index;
        bool m_hasSpyPoints;
        unsigned int m_trackSubType;
        enum MissionMode m_missionMode;
        int m_difficulty;
        unsigned int m_truckFrequencyType;
        unsigned int unused1;
        unsigned int m_clearanceLevelIndex;
        char* m_missionTitle;
        char* m_missionDescription;
        char* m_difficultyStr;
        unsigned int m_lapCount;
        unsigned int m_maxLapCount;
        float m_missionTimeSeconds;
        unsigned int m_killCount;
        unsigned int m_friendlyFireType;
        bool m_weaponsEnabled;
        AllowedWeaponList m_weaponsList;
        unsigned int m_aiCarCount;
        unsigned int m_fieldAgentCount;
        TakedownSettings m_takeDownSettings;
        float m_aiSkillMin4;
        float m_aiSkillMax4;
        float m_aiSkillMin3;
        float m_aiSkillMax3;
        float m_aiSkillMin2;
        float m_aiSkillMax2;
        float m_aiSkillMin;
        float m_aiSkillMax;
        char* m_startingCarId[10];
        char* m_startingCarName[10];
        char* m_startingCarModelName[10];
        CarAIOverrideParameters m_aiOverrideParameters[10];
    };

	static inline MissionModeInfo* s_MissionModeInfos = reinterpret_cast<MissionModeInfo*>(0x0184cea0);
public:
	char unk[0xB8];
	int m_unkIndex;
    int m_unk2;
    struct MissionSettings m_settings;
    struct MissionSettings m_squadSeriesSettings[6];
    char* m_unk3;
    unsigned int m_squadSeriesIndex;
    unsigned int m_auroraCategory;
    CMessageOwner m_messageOwner;
    GenericMessageHandler<GameProgressionManager> m_persistentDataResetToDefaultsHandler;
    GenericMessageHandler<GameProgressionManager> m_requestSwitchToLevelHandler;
    GenericMessageHandler<GameProgressionManager> m_requestQuitGameHandler;
    unsigned int m_unk4;
    CheatState m_cheatStates[std::to_underlying(Cheat::Max)];
public:
	inline char* FormatStoryMission(int clearance_level, int mission_index) {
		return GameProgressionManager_FormatStoryMission(this, clearance_level, mission_index);
	}
	
    inline MissionMode GetMissionMode() const {
		return *reinterpret_cast<MissionMode*>(reinterpret_cast<std::uintptr_t>(this) + 0xF8);
	}
    
    inline bool QueryCheatActive(Cheat cheat) const {
        return m_cheatStates[std::to_underlying(cheat)] == CheatState::Active;
    }
    
    inline int GetAICarCount() {
        return GameProgressionManager_GetAICarCount(this);
    }

    inline void SetAICarCount(int count) {
        GameProgressionManager_SetAICarCount(this, count);
    }

    static inline constexpr std::string_view CheatName(Cheat cheat) {
        constexpr std::array<std::string_view, std::to_underlying(GameProgressionManager::Cheat::Max)> cheatIndexToName = {
            "Cheat_Max_Energy",
            "Cheat_Unlimited_Energy",
            "Cheat_Start_MachineGuns",
            "Cheat_Start_WGP",
            "Cheat_Unlock_PlayableCars",
            "Cheat_Unlock_GameModes",
            "Cheat_Unlock_AllTracks"
        };
        return cheatIndexToName[std::to_underlying(cheat)];
    }
};

static_assert(sizeof(GameProgressionManager) == 0x1d40);

inline GameProgressionManager** g_GameProgressionManager = reinterpret_cast<GameProgressionManager**>(0x018ae0f0);