#pragma once
#include "DBlock.hpp"
#include "../CActor.hpp"
#include "../Components/CarsVehiclePlayer.hpp"
#include "../Components/ActiveMoves.hpp"
#include "../Components/CarsReactionMonitor.hpp"

namespace Animation {
	class DataNode;
};

class Cars2VehicleDBlock : public DBlock {
public:
	enum class NodeType {
		Body = 0,
		BodyCtrl,
		MainCtrl,
		FxMarkerSkateJack,
		FxMarkerMachineGunLeft,
		FxMarkerMachineGunRight,
		FxMarkerWGPCam,
		FxMarkerExhaustRight1,
		FxMarkerExhaustLeft1,
		FxMarkerExhaustRight2,
		FxMarkerExhaustLeft2,
		FxMarkerHeadLightRight,
		FxMarkerHeadLightLeft,
		PhysicsOffsetCtrl,
		BodyFinalProxy,
		RbMain,
		PhysicsMainCtrl,
		ToWorldMainCtrl,
		ToWorldBody,
		ToWorldBodyCtrl,
		ToWorldFxMarkerSkateJack,
		ToWorldFxMarkerWGPCam,
		Max
	};
	CarsVehicle* m_carsVehicle;
	struct LapTracker* m_lapTracker;
	ActiveMoves* m_activeMoves;
	CarsVehiclePlayer* m_carsVehiclePlayer;
	CarsControlMapper* m_carsControlMapper;
	CarsReactionMonitor* m_carsReactionMonitor;
	Animation::DataNode* m_dataNodes[22];
	int m_avatarVehicle;
	int m_playerNum;
	bool m_visibleToCamera;
	float m_closestCameraDistance;
public:
	inline static Cars2VehicleDBlock* Get(CActor& actor) {
		return reinterpret_cast<Cars2VehicleDBlock*>(actor.GetDataBlock(DBlock::BlockType::Cars2Vehicle));
	}
};

static_assert(sizeof(Cars2VehicleDBlock) == 144);