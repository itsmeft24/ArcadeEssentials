#include "../../CMessage.hpp"
#include "../../Utils/Vector3.hpp"
#include "../../Utils/Matrix.hpp"
#include "../../DynamicArray.hpp"

using FourCC = int;

struct ts_MinionState {
    ActorHandle handle;
    int state;
    float stateTimer;
    int health;
    unsigned int warpInChoHandle;
    int weapon;
    int weaponState;
    float weaponTimer;
    float explodeTime;
    bool explosive;
    bool explodedOnDeath;
    bool spawning;
    ActorHandle killedBy;
    float hideTime;
    float spawnTime;
    Vector3 spawnPosition;
    Matrix3x3 spawnOrientation;
    int chevronColor;
};

struct ts_PlayerWarpState {
    int state;
    float timer;
};

struct MissionPointInfo {
    ActorHandle m_scorer;
    ActorHandle m_carKilled;
    int m_pointValue;
    float m_timeStamp;
};

struct ts_SpawnLocator {
    Vector3 position;
    float heading;
    float trackPosition;
    float triggerDistance;
};

constexpr auto MAX_MINION_CARS = 10;
constexpr auto MAX_HUMAN_PLAYERS = 4;
constexpr auto MAX_RACERS_PER_TEAM = 8;
constexpr auto MAX_TEAMS = 4;
constexpr auto INVALID_SORTING_SCORE = -1;

class MissionGameInf {
public:
    enum class Team {
        Invalid = -1,
        Red = 0,
        Blue = 1,
        Green = 2,
        Yellow = 3,
        Max = 4,
    };

    enum class WeaponType {
        Unarmed = 0,
        SpareTire,
        LeechMine,
        MachineGun,
        Missile,
        SkateJack,
        Max,
    };

    enum class WeaponState {
        StartDelay = 0,
        WaitForLoadOut = 1,
        CheckDriveBackwards = 2,
        DriveBackwardsActive = 3,
        FiringWeapon = 4,
        DriveBackwardsCheckDelay = 5,
        PassiveDrive = 6,
    };
protected:
    ts_MinionState m_minions[MAX_MINION_CARS];
    int m_minionCarCount;
    bool m_weaponCanBePassive;
    CMessageOwner m_messageOwner;
    int m_playerScore[MAX_HUMAN_PLAYERS];
    int m_teamScores[MAX_TEAMS];
    bool m_teamScoreLocked[MAX_TEAMS];
    bool m_teamActive[MAX_TEAMS];
    int m_teamSize[MAX_TEAMS];
    ActorHandle m_teamRacers[MAX_TEAMS][MAX_RACERS_PER_TEAM];
    int m_teamSortingScores[MAX_TEAMS];
    int m_scoreToWin[MAX_TEAMS];
    bool m_stopGameWithTimer;
    bool m_delayGameEnd;
    float m_maxTimeRemaining;
    float m_timeRemainingMultiplier[MAX_TEAMS];
    float m_timeLeft[MAX_TEAMS];
    int m_timeLeftSeconds[MAX_TEAMS];
    bool m_timerLocked[MAX_TEAMS];
    int m_originalTeam[MAX_TEAMS];
    float m_initialDelay;
    float m_driveBackwardsChance;
    float m_driveBackwardsDuration;
    float m_driveBackwardsDelay;
    DynamicArray<MissionPointInfo> m_missionPointInfoObjs;
    bool m_gameActive;
    bool m_raceActive;
    bool m_notifyLeadOfChange;
    bool m_playCountdownVO;
    bool m_playCountdownWarningBeeps;
    int m_sortedPlayerID[MAX_HUMAN_PLAYERS];
    bool m_playerResultsSorted;
    DynamicArray<ts_SpawnLocator> m_spawnLocatorList;
    int m_initializeWait;
    ts_PlayerWarpState m_playerWarp[MAX_HUMAN_PLAYERS];
    GenericMessageHandler<MissionGameInf> m_hudInitialized;
    GenericMessageHandler<MissionGameInf> m_incrementScore;
    GenericMessageHandler<MissionGameInf> m_startMission;
    bool m_needHealthTracker;
    bool m_endOnScore;
    bool m_missionEvaluated;
    bool m_timerPaused;
    bool m_disableMinionAttack;
    int m_leader;
    int m_leaderHandle;
    bool m_wasFiredCountdown10;
    bool m_wasFiredCountdown5;
    bool m_wasFiredCountdown0;
    bool m_attachBeacons;
public:
    virtual ~MissionGameInf();
    virtual void Update();
    virtual void GetWeaponType(int* weapon);
    virtual void AdjustTimeRemaining(float adjTime, Team team = Team::Invalid, bool applyModifier = true);
    virtual void UpdateHUDAllPlayers();
    virtual void UpdateHUDWithTime();
    virtual void Add1Up(ActorHandle car, int score);
    virtual void TimeComplete();
    virtual void Cleanup();
    virtual void RestartRace();
    virtual bool CarHitByWeapon(const ActorHandle i_victim, const ActorHandle i_shooter, const FourCC& munition);
    virtual void PickupCollected(const AgentID collectedPickupAgentID, const ActorHandle playerHandle);
    virtual void MissionSpecificGameFinished(int winningTeam);
    virtual void SetupMissionHUD();
    virtual void SetupMarkers(int player);
    virtual void AddHealthTrackers();
    virtual void AddMissionPointInfo(MissionPointInfo* mpi);
    virtual void CheckForActiveTeams();
    virtual void SetMissionControlContext(bool enable);
};

