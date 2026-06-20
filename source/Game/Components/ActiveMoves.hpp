#pragma once
#include <cstdint>
#include <cmath>
#include "../CarManager.hpp"
#include "../Components/CActorComponent.hpp"
#include "../Components/CarsVehicle.hpp"
#include "../../Game/Types.hpp"
#include "../../Game/Utils/PIDController.hpp"
#include "../../Game/Utils/Vector3.hpp"
#include "../../Game/Physics/RigidBody.hpp"

inline auto ActiveMoves_SendSideBashMunitionImpact = (void(__thiscall*)(class ActiveMoves*, ActorHandle))(0x006fac50);
inline auto ActiveMoves_SetTwoWheelingLeft = (void(__thiscall*)(class ActiveMoves*, bool))(0x006f9aa0);
inline auto ActiveMoves_SetTwoWheelingRight = (void(__thiscall*)(class ActiveMoves*, bool))(0x006f9be0);
inline auto ActiveMoves_SetSideSteppingLeft = (void(__thiscall*)(class ActiveMoves*, bool))(0x006f9d20);
inline auto ActiveMoves_SetSideSteppingRight = (void(__thiscall*)(class ActiveMoves*, bool))(0x006f9ee0);
inline auto ActiveMoves_SetDriftingLeft = (void(__thiscall*)(class ActiveMoves*, bool, bool))(0x006fa880);
inline auto ActiveMoves_SetDriftingRight = (void(__thiscall*)(class ActiveMoves*, bool, bool))(0x006faa50);
inline auto ActiveMoves_SetBackwardsDriving = (void(__thiscall*)(class ActiveMoves*, bool))(0x006fa090);
inline auto ActiveMoves_SetBunnyHopping = (void(__thiscall*)(class ActiveMoves*, bool, bool))(0x006f9860);

class ActiveMoves : public CActorComponent {
public: 
	enum ActionState : int {
		None,
		BunnyHop,
		TwoWheelLeft,
		TwoWheelRight,
		SideStepLeft,
		SideStepRight,
		BackwardsDriving,
		JumpTrick,
		DriftLeft,
		DriftRight,
		Max,
	};
	enum class JumpState : int {
		None,
		PreLoad,
		Launch,
		InAir,
	};
	enum class JumpType : int {
		BunnyHop,
		SideStepLeft,
		SideStepRight,
		Max,
	};
	enum class JumpTrickState : int {
		None,
		Idle,
		SpinLeft,
		SpinRight,
		RollLeft,
		RollRight,
		FlipUp,
		FlipDown,
		RevIdle,
		RevSpinLeft,
		RevSpinRight,
		RevRollLeft,
		RevRollRight,
		RevFlipUp,
		RevFlipDown,
		Max
	};
	enum class JumpTrickType : int {
		Stop,
		Start,
		SpinLeft,
		SpinRight,
		RollLeft,
		RollRight,
		FlipUp,
		FlipDown,
		Max
	};
	enum class DriftWallRideType : int {
		Search,
		Sighted,
		Approach,
		Contact,
		Leaving,
	};
	enum class TauntState : int {
		None,
		Pending,
		Start,
		Taunting,
		Cooldown,
	};
	enum class MoveHierState : int {
		None,
		BunnyHop,
		TwoWheel,
		WallRide,
		SideStep,
		BackwardsDriving,
		AirTricks,
		Drift,
		Max,
	};
	struct MoveHierStateNameValue {
		const char* name;
		MoveHierState value;
	};
public:
	bool m_isSetup;
	bool m_toggleStickToGround;
	bool m_isAIControlled;
	bool m_inAirFromBunnyHopping;
	bool m_bunnyHopIsWiimote;
	float m_bunnyHopForce;
	int m_nCompletedTricks;
	CarManager::ActiveMovesSettings* m_activeMovesSettings;
	CarManager::SteeringSettings* m_steeringSettings;
	CarManager::GameSettings* m_gameSettings;
	float m_sideStepUpForce;
	float m_sideStepSideForce;
	float m_jumpPreloadTime;
	float m_jumpTriggerTime;
	float m_jumpElapsedTime;
	float m_backwardsDrivingTimer;
	float m_turboDuration;
	float m_turboTime;
	bool m_turboing;
	bool m_honorTurboDuration;
	std::uint32_t m_turboSfxHandle;
	Physics::RigidBody* m_rBody;
	Vector3 m_worldUp;
	Vector3 m_carRight;
	Vector3 m_carUp;
	Vector3 m_carFwd;
	void* m_line;
	ActionState	m_actionState;
	CarsVehicle* m_carsVehicleComponent;
	bool m_lastOnGround;
	bool m_crashing;
	bool m_crashCameraLockout;
	JumpState m_jumpState;
	JumpType m_jumpType;
	float m_jumpUpForce;
	float m_jumpSideForce;
	JumpTrickState m_jumpTrickState;
	JumpTrickType m_jumpTrickPendingAction;
	JumpTrickType m_jumpTrickPreviousAction;
	float m_jumpTrickTime;
	float m_jumpTrickAccumulatedEnergy;
	int m_jumpTrickEnergyRate;
	int m_jumpTrickEnergyMaxRate;
	float m_jumpTrickFailStartAngle;
	float m_jumpTrickFailStopAngle;
	float m_jumpTrickFailTime;
	bool m_jumpTrickFailRespawnPending;
	bool m_jumpTrickActionIsPending;
	bool m_jumpTrickLoopRight;
	bool m_jumpTrickLoopLeft;
	bool m_jumpTrickLoopUp;
	bool m_jumpTrickLoopDown;
	bool m_jumpTrickRightOn;
	bool m_jumpTrickLeftOn;
	bool m_jumpTrickUpOn;
	bool m_jumpTrickDownOn;
	bool m_jumpTrickIsGood;
	bool m_jumpTrickIdleSpinLeft;
	bool m_jumpTrickLanding;
	bool m_jumpTrickFail;
	bool m_jumpTrickFailingSpin;
	bool m_jumpTrickFailingFlip;
	bool m_jumpTrickFailingRoll;
	bool m_jumpTrickFailJumped;
	bool m_jumpTrickFailCrash;
	Vector3 m_jumpTrickFailCrashVel;
	DriftWallRideType m_driftWallRideState;
	bool m_driftWallRidingLeft;
	bool m_driftWallRidingRight;
	bool m_driftWallRideFeelerHit;
	bool m_driftWallRideInContact;
	bool m_driftWallRideContacted;
	bool m_driftWallRideCollisionDisabled;
	bool m_driftWallRideApproachingWall;
	bool m_driftWallRideLeavingWall;
	uint8_t m_driftWallRideNearingHistory;
	Vector3 m_driftWallRideCarPos;
	Vector3 m_driftWallRideNearestWallPoint;
	Vector3 m_driftWallRideNearestWallNormal;
	Vector3 m_driftWallRidePreviousQueryEnd;
	float m_driftWallRideETA;
	float m_driftWallRideETATimer;
	float m_driftWallRideDistToWallPrev;
	float m_driftWallRideDistToWall;
	float m_driftWallRideSlipAngleFactor;
	float m_twoWheelingTime;
	float m_twoWheelingAirTime;
	float m_twoWheelBalance;
	float m_twoWheelBalanceVel;
	float m_twoWheelOffBalanceTime;
	bool m_twoWheelOffBalance;
	bool m_twoWheelIndecisionGoesLeft;
	bool m_twoWheelSlow;
	PIDController<20> m_twoWheelBalancePIDController;
	float* m_twoWheelBalanceParam;
	float m_driftRedoTimeLeft;
	float m_bashTimer;
	TauntState m_tauntState;
	int m_tauntLastRank;
	float m_tauntStateTimer;
	void* m_audioComponent;
	bool m_playIdleOnStopAction;
	bool m_carResetting;
	float m_controlsDisabledTimer;
	bool m_bashStatCredited;
	// To be continued...
public:
	inline Physics::RigidBody* GetRigidBody() {
		return m_rBody;
	}
	inline bool GetTurboing() const {
		return m_turboing;
	}
	inline bool GetInAirFromBunnyHop() const {
		return m_inAirFromBunnyHopping;
	}
	inline bool GetBunnyHopping() const {
		return m_actionState == ActionState::BunnyHop;
	}
	inline bool GetSideSteppingLeft() const {
		return m_actionState == ActionState::SideStepLeft;
	}
	inline bool GetSideSteppingRight() const {
		return m_actionState == ActionState::SideStepRight;
	}
	inline bool GetSideStepping() const {
		return GetSideSteppingLeft() || GetSideSteppingRight();
	}
	inline bool GetTwoWheelingLeft() const {
		return m_actionState == ActionState::TwoWheelLeft;
	}
	inline bool GetTwoWheelingRight() const {
		return m_actionState == ActionState::TwoWheelRight;
	}
	inline bool GetTwoWheeling() const {
		return GetTwoWheelingLeft() || GetTwoWheelingRight();
	}
	inline bool GetBackwardsDriving() const {
		return m_actionState == ActionState::BackwardsDriving;
	}
	inline bool GetDriftingLeft() const {
		return m_actionState == ActionState::DriftLeft;
	}
	inline bool GetDriftingRight() const {
		return m_actionState == ActionState::DriftRight;
	}
	inline bool GetDrifting() const {
		return GetDriftingLeft() || GetDriftingRight();
	}
	inline void SendSideBashMunitionImpact(ActorHandle handle) {
		ActiveMoves_SendSideBashMunitionImpact(this, handle);
	}
	inline void SetTwoWheelingLeft(bool two_wheeling_left) {
		ActiveMoves_SetTwoWheelingLeft(this, two_wheeling_left);
	}
	inline void SetTwoWheelingRight(bool two_wheeling_right) {
		ActiveMoves_SetTwoWheelingRight(this, two_wheeling_right);
	}
	inline void SetBackwardsDriving(bool bwd) {
		ActiveMoves_SetBackwardsDriving(this, bwd);
	}
	inline void SetSideSteppingLeft(bool ssl) {
		ActiveMoves_SetSideSteppingLeft(this, ssl);
	}
	inline void SetSideSteppingRight(bool ssr) {
		ActiveMoves_SetSideSteppingRight(this, ssr);
	}
	inline void SetDriftingLeft(bool drl, bool autoDrift) {
		ActiveMoves_SetDriftingLeft(this, drl, autoDrift);
	}
	inline void SetDriftingRight(bool drr, bool autoDrift) {
		ActiveMoves_SetDriftingRight(this, drr, autoDrift);
	}
	inline void SetBunnyHopping(bool bh, bool forceHop) {
		ActiveMoves_SetBunnyHopping(this, bh, forceHop);
	}
};

bool _fastcall TriggerSideBashReactions(ActiveMoves* _this, std::uintptr_t edx, ActorHandle victim, bool bashTie);