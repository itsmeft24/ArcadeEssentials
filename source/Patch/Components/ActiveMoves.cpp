#include "../../Game/CActor.hpp"
#include "../../Game/Components/ActiveMoves.hpp"
#include "../../Game/Components/CarsVehicle.hpp"
#include "../../Game/CarManager.hpp"
#include "../../Game/Stage/Cars2VehicleDBlock.hpp"
#include "../../Game/GameProgressionManager.hpp"
#include "../../Game/Physics/RigidBody.hpp"
#include "../../Game/Utils/AnimEventDispatcher.hpp"
#include "../../Game/Stage/ActorCommands.hpp"
#include "../../Game/CMessage.hpp"
#include "../../Game/CarsAIManager.hpp"
#include "../../Game/Avatar.hpp"
#include "../../Game/CarsCamManager.hpp"
#include "../../Game/Hud/CarsHud.hpp"

bool _fastcall TriggerSideBashReactions(ActiveMoves* _this, std::uintptr_t edx, ActorHandle victim, bool bashTie) {
	CActor* victimPtr = CActor::FromHandle(victim);
	if (victimPtr == nullptr) {
		return false;
	}

	ActiveMoves* pAM = Cars2VehicleDBlock::Get(*victimPtr)->m_activeMoves;
	if (pAM == nullptr) {
		return false;
	}

	Physics::RigidBody* victimRBody = pAM->GetRigidBody();
	if (victimRBody == nullptr) {
		return false;
	}

	if (_this->m_bashTimer > 0.0f || pAM->m_bashTimer > 0.0f) {
		return false;
	}

	_this->m_bashTimer = 0.05f;
	pAM->m_bashTimer = 0.05f;

	Vector3 victimDir = victimPtr->GetBodyInWorld() - _this->GetActor().GetBodyInWorld();
	float rightness = Vector3::Dot(victimDir, _this->m_carRight);

	Vector3 myVel = _this->m_rBody->GetVelocity();
	Vector3 hisVel = victimRBody->GetVelocity();
	Vector3 carRightInXZ = _this->m_carRight;
	carRightInXZ.Y(0);
	carRightInXZ.Normalize();

	const float sideBashSpeed = 34.3f;
	Vector3 myVelProjectedToMyRight = sideBashSpeed * _this->m_carRight;

	bool victimWas2WDorBWD = pAM->GetBackwardsDriving() || pAM->GetTwoWheeling();
	bool sendSideBashMI = false;

	if (_this->GetSideSteppingLeft() && rightness <= 0.0f) {
		AnimEventDispatcher::SendEvent("React_Bash_Impacter_L", _this->GetActor().GetHandle());
		AnimEventDispatcher::SendEvent("React_Bash_Impactee_R", victimPtr->GetHandle());
		sendSideBashMI = true;
		CarsVehicle::BumpBashHandle(&_this->GetActor(), victimPtr, true);
		if (bashTie) {
			CarsVehicle::BumpBashHandle(victimPtr, &_this->GetActor(), true);
		}
	}
	else if (_this->GetSideSteppingRight() && rightness >= 0.0f) {
		AnimEventDispatcher::SendEvent("React_Bash_Impacter_R", _this->GetActor().GetHandle());
		AnimEventDispatcher::SendEvent("React_Bash_Impactee_L", victimPtr->GetHandle());
		sendSideBashMI = true;
		CarsVehicle::BumpBashHandle(&_this->GetActor(), victimPtr, true);
		if (bashTie) {
			CarsVehicle::BumpBashHandle(victimPtr, &_this->GetActor(), true);
		}
	}
	else {
		return false;
	}

	if (!bashTie) {
		// _this->m_carsVehicleComponent->GetCarEnergy().AddEnergy(_this->m_gameSettings->bashingReward, true, true, 0, nullptr, -1, false);
		_this->m_carsVehicleComponent->GetCarEnergy().AddEnergy(*reinterpret_cast<float*>(reinterpret_cast<std::uintptr_t>(_this->m_gameSettings) + 0xA0), true, true, 0, nullptr, -1, false);
		if (_this->m_carsVehicleComponent->GetInTheZone()) {
			if ((*g_GameProgressionManager)->GetMissionMode() == GameProgressionManager::MissionMode::Takedown)
				ActorCommands::DamageActor(victim, 100.0f, "blsh", &_this->GetActor());
			else
				ActorCommands::DamageActor(victim, 100.0f, "rcsj", &_this->GetActor());
			return true;
		}
		else if (victimWas2WDorBWD) {
			(*g_MessageDispatcher)->SendMessageToActor("ForceWipeout", 0, victim, victim);
		}
		(*g_CarsHud)->TrickCallout(_this->actor, "Battery", "CallOut_Side_Bash", 1);
	}

	uint32_t myCarId = (*g_CarManager)->GetCarId(_this->GetActor().GetHandle());
	uint32_t victimCarId = (*g_CarManager)->GetCarId(victimPtr->GetHandle());

	CarManager::CarClass myCarClass = (*g_CarManager)->GetCarClass(myCarId);
	CarManager::CarClass victimCarClass = (*g_CarManager)->GetCarClass(victimCarId);

	int myBashWeight = 1;
	if (myCarClass == CarManager::CarClass::Light) myBashWeight = 0;
	if (myCarClass == CarManager::CarClass::Heavy) myBashWeight = 2;

	int victimBashWeight = 1;
	if (victimCarClass == CarManager::CarClass::Light) victimBashWeight = 0;
	if (victimCarClass == CarManager::CarClass::Heavy) victimBashWeight = 2;

	int bashWeightDifference = myBashWeight - victimBashWeight;

	float throwSideAttackerClassFactor = 1.0f;
	float throwSideVictimClassFactor = 1.0f;
	float throwUpVictimClassFactor = 1.0f;

	switch (bashWeightDifference)
	{
	case -2:
		throwSideAttackerClassFactor = 1.0f / (_this->m_activeMovesSettings->sideStepBashThrowSideAttackerClassFactor * _this->m_activeMovesSettings->sideStepBashThrowSideAttackerClassFactor);
		throwSideVictimClassFactor = 1.0f / (_this->m_activeMovesSettings->sideStepBashThrowSideVictimClassFactor * _this->m_activeMovesSettings->sideStepBashThrowSideVictimClassFactor);
		throwUpVictimClassFactor = 1.0f / (_this->m_activeMovesSettings->sideStepBashThrowUpVictimClassFactor * _this->m_activeMovesSettings->sideStepBashThrowUpVictimClassFactor);
		break;
	case -1:
		throwSideAttackerClassFactor = 1.0f / _this->m_activeMovesSettings->sideStepBashThrowSideAttackerClassFactor;
		throwSideVictimClassFactor = 1.0f / _this->m_activeMovesSettings->sideStepBashThrowSideVictimClassFactor;
		throwUpVictimClassFactor = 1.0f / _this->m_activeMovesSettings->sideStepBashThrowUpVictimClassFactor;
		break;
	case 0:
		throwSideAttackerClassFactor = 1.0f;
		throwSideVictimClassFactor = 1.0f;
		throwUpVictimClassFactor = 1.0f;
		break;
	case 1:
		throwSideAttackerClassFactor = _this->m_activeMovesSettings->sideStepBashThrowSideAttackerClassFactor;
		throwSideVictimClassFactor = _this->m_activeMovesSettings->sideStepBashThrowSideVictimClassFactor;
		throwUpVictimClassFactor = _this->m_activeMovesSettings->sideStepBashThrowUpVictimClassFactor;
		break;
	case 2:
		throwSideAttackerClassFactor = _this->m_activeMovesSettings->sideStepBashThrowSideAttackerClassFactor * _this->m_activeMovesSettings->sideStepBashThrowSideAttackerClassFactor;
		throwSideVictimClassFactor = _this->m_activeMovesSettings->sideStepBashThrowSideVictimClassFactor * _this->m_activeMovesSettings->sideStepBashThrowSideVictimClassFactor;
		throwUpVictimClassFactor = _this->m_activeMovesSettings->sideStepBashThrowUpVictimClassFactor * _this->m_activeMovesSettings->sideStepBashThrowUpVictimClassFactor;
		break;
	}
	
	float throwSideMe;
	float throwSideVictim;
	float throwUpVictim;

	if (_this->GetTurboing())
	{
		throwSideMe = _this->m_activeMovesSettings->sideStepBashTurboThrowSideAttacker;
		throwSideVictim = _this->m_activeMovesSettings->sideStepBashTurboThrowSideVictim;
		throwUpVictim = _this->m_activeMovesSettings->sideStepBashTurboThrowUpVictim;
	}
	else
	{
		throwSideMe = _this->m_activeMovesSettings->sideStepBashThrowSideAttacker;
		throwSideVictim = _this->m_activeMovesSettings->sideStepBashThrowSideVictim;
		throwUpVictim = _this->m_activeMovesSettings->sideStepBashThrowUpVictim;
	}

	float throwSideTie = 0.5f * (throwSideMe + throwSideVictim);

	throwSideMe *= throwSideAttackerClassFactor;
	throwSideVictim *= throwSideVictimClassFactor;
	throwUpVictim *= throwUpVictimClassFactor;
	
	Vector3 myNewVel;
	Vector3 hisNewVel;
	float throwDir = (rightness < 0) ? -1.0f : 1.0f;

	if (bashTie) {
		myNewVel = myVel - throwDir * (myVelProjectedToMyRight + carRightInXZ * throwSideTie);
		hisNewVel = hisVel + throwDir * (myVelProjectedToMyRight + carRightInXZ * throwSideTie) + Vector3(0, 1, 0) * throwUpVictim;
	}
	else {
		myNewVel = myVel - throwDir * (myVelProjectedToMyRight + carRightInXZ * throwSideMe);
		hisNewVel = hisVel + throwDir * (myVelProjectedToMyRight + carRightInXZ * throwSideVictim) + Vector3(0, 1, 0) * throwUpVictim;
	}

	_this->m_rBody->SetVelocity(myNewVel);
	_this->m_rBody->SetLocalAngularVelocity(Vector3(0, 0, 0));

	uint32_t myRumbleDurationMS = 250;
	uint32_t victimRumbleDurationMS = 250;
	int myRumbleMagnitude = 128;
	int victimRumbleMagnitude = 128;
	if (bashTie) {
		victimRBody->SetVelocity(hisNewVel);
	}
	else {
		victimRBody->SetVelocity(hisNewVel * _this->m_activeMovesSettings->sideStepBashPenaltyAmount);
		victimRumbleDurationMS = 750;
		victimRumbleMagnitude = 255;
		(*g_CarsAIManager)->HandleSideBash(_this->actor->GetHandle(), victim);
	}
	victimRBody->SetLocalAngularVelocity(Vector3(0, 0, 0));

	Avatar* myAvatar = Avatar::FromCActor(_this->actor);
	if (myAvatar && myAvatar->IsPlayerControlled()) {
		CarsControlMapper* ccm = Cars2VehicleDBlock::Get(*_this->actor)->m_carsControlMapper;
		if (ccm != nullptr) ccm->SetVibrationDuration(myRumbleDurationMS, myRumbleMagnitude);
	}

	Avatar* victimAvatar = Avatar::FromCActor(victimPtr);
	if (victimAvatar && victimAvatar->IsPlayerControlled()) {
		int playerId = victimAvatar->GetPlayerId();
		(*g_CarsCamManager)->StartEyeShake(playerId, 0.8f, 10.0f, static_cast<float>(victimRumbleDurationMS) / 1000.0f, true, Vector3(1.0f, 1.0f, 1.0f));
		CarsControlMapper* ccm = Cars2VehicleDBlock::Get(*_this->actor)->m_carsControlMapper;
		if (ccm != nullptr) ccm->SetVibrationDuration(victimRumbleDurationMS, victimRumbleMagnitude);
	}

	if (sendSideBashMI) {
		_this->SendSideBashMunitionImpact(victimPtr->GetHandle());
	}

	if ((*g_GameProgressionManager)->GetMissionMode() == GameProgressionManager::MissionMode::Tutorial && sendSideBashMI) {
		Avatar* pAvatar = Avatar::FromCActor(_this->actor);
		if (pAvatar && pAvatar->IsPlayerControlled()) {
			(*g_MessageDispatcher)->SendMessageToAll("BashedByPlayer", nullptr, victim);
		}
	}

	return true;
}
