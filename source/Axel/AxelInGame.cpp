#include <sunset.hpp>
#include "Axel.hpp"
#include "AxelMessageTypes.hpp"
#include "../Game/GameProgressionManager.hpp"
#include "../Game/MiscOffsets.hpp"
#include "../pentane.hpp"
#include "../Game/GamePlay/Race/RaceManager.hpp"
#include "../Game/Components/ActiveMoves.hpp"
#include "../Game/Components/CarsReactionMonitor.hpp"
#include "../Game/Hud/CarsHud.hpp"

using namespace std::chrono_literals;

// Configures the starting grid location index for each player.
// For now, we cheaply synchronize them by returning the Axel IDs, since those will be synchronized.
DefineReplacementHook(GetStartingGridLocForPlayer) {
	static int _fastcall callback(void* _this, std::uintptr_t edx, int playerId) {
		if (!axel::online()) {
			return original(_this, edx, playerId);
		}
		return axel::to_axel_id(playerId);
	}
};

// Stubs the function responsible for placing the first starting car, so our `FillStartingGrid` implementation can handle it.
DefineReplacementHook(SetStartingCarId) {
	static void __fastcall callback(GameProgressionManager * _this, std::uintptr_t edx, int index, char* carId, bool replaceEntry) {
		if (!axel::online()) {
			original(_this, edx, index, carId, replaceEntry);
			return;
		}
	}
};

// Fills out the starting grid with the selected players, and forces the AI car count to zero.
DefineReplacementHook(FillStartingGrid) {
	static void _fastcall callback(GameProgressionManager * _this) {
		if (!axel::online()) {
			original(_this);
			return;
		}

		_this->m_settings.m_aiCarCount = 0;
		for (auto axelId = 0; axelId < 10; axelId++) {
			if (axelId < axel::player_count()) {
				int localId = axel::CONTEXT->lobbyMembers[axelId].playerId;
				char* evaluated = const_cast<char*>("McQueen");
				if (localId == 0) {
					unsigned int unknown = 0;
					void** entry = GameEnv_GetWorldData(*g_GameEnv, &unknown, "DefaultAvatar", -2);
					if (*entry != nullptr) {
						evaluated = SFD_Entry_GetValue(*entry, 0);
					}
				}
				else {
					auto defaultAvatarStr = std::format("DefaultAvatar{}", localId + 1);
					unsigned int unknown = 0;
					void** entry = GameEnv_GetWorldData(*g_GameEnv, &unknown, defaultAvatarStr.data(), -2);
					if (*entry != nullptr) {
						evaluated = SFD_Entry_GetValue(*entry, 0);
					}
				}

				char* modelName = UnkDatabaseExcel_GetModelNameForCarId(*g_UnkDatabaseExcel, evaluated);
				_this->m_settings.m_startingCarId[axelId] = const_cast<char*>("PLAYER");
				_this->m_settings.m_startingCarModelName[axelId] = modelName;
				_this->m_settings.m_startingCarName[axelId] = evaluated;
			}
		}

		for (std::size_t i = 0; i < 10; i++) {
			std::string_view carId = _this->m_settings.m_startingCarId[i] != nullptr ? _this->m_settings.m_startingCarId[i] : "";
			std::string_view carModelName = _this->m_settings.m_startingCarModelName[i] != nullptr ? _this->m_settings.m_startingCarModelName[i] : "";
			std::string_view carName = _this->m_settings.m_startingCarName[i] != nullptr ? _this->m_settings.m_startingCarName[i] : "";
			logger::log_format("[GameProgressionManager::FillStartingGrid] Location: {}, CarId: {}, CarModel: {}, CarName: {}", i, carId, carModelName, carName);
		}

		// We need to check for duplicates, otherwise `CarsPlayers::ReadParamSet` will erroneously
		// add `instanceMethod` and `instancingFlags` to both players' `ParameterSets`. This avoids
		// an infinite-load.
		for (std::size_t i = 0; i < axel::player_count(); i++) {
			for (std::size_t j = 0; j < axel::player_count(); j++) {
				if (i != j && _stricmp(_this->m_settings.m_startingCarName[i], _this->m_settings.m_startingCarName[j]) == 0) {
					logger::log_format("[GameProgressionManager::FillStartingGrid] Location: {} has duplicate!", i);
					_this->m_settings.m_aiOverrideParameters[i].hasCharacterDuplicate = true;
					break;
				}
			}
		}
	}
};

// Sends the `DoneLoading` packet right after world assets are loaded, and delays the loading screen
// until all players are ready to start the race.
DefineReplacementHook(ArcadeManager_CanStart) {
	static bool __fastcall callback(void* _this) {
		if (!axel::online()) {
			return true;
		}

		// If we just finished loading, we need to tag the global context as such/send out messages as needed.
		if (!axel::CONTEXT->isMemberReady[axel::CONTEXT->myAxelId]) {
			// If we are not the party leader, we need to tell the party leader that we finished loading the world assets.
			if (!axel::CONTEXT->isHost) {
				auto doneLoading = axel::message::DoneLoadingPacket::make(axel::CONTEXT->myAxelId, 0);
				std::vector<std::uint8_t> packetBytes(sizeof(axel::message::DoneLoadingPacket));
				std::memcpy(packetBytes.data(), &doneLoading, sizeof(axel::message::DoneLoadingPacket));
				axel::CONTEXT->network->send_routed_message_async(axel::CONTEXT->lobbyMembers[0].steamId, packetBytes);
			}
			// We need to manually set our load state, otherwise this hook will keep trying to send packets out.
			axel::CONTEXT->isMemberReady[axel::CONTEXT->myAxelId] = true;
			return false;
		}
		else {
			if (axel::CONTEXT->isHost) {
				// If we're the party leader, we care about `isMemberReady`; we need everyone to have told us they're ready to go.
				if (!axel::CONTEXT->allOpponentsReady) {
					// This block will stop being executed once all opponents have told us they're ready, so we can safely send the `StartGame` packet here.
					for (auto i = 0; i < axel::CONTEXT->playerCount; i++) {
						if (!axel::CONTEXT->isMemberReady[i]) {
							return false;
						}
					}
					logger::log_format("[ArcadeManager::NotWaiting] All opponents finished loading.");

					axel::CONTEXT->allOpponentsReady = true;

					// Compute a start time sufficiently in the future; five seconds should be long enough!
					std::chrono::utc_clock::time_point startTime = std::chrono::utc_clock::now() + 1s;
					axel::CONTEXT->startTime = std::chrono::utc_clock::now() + 1s;

					for (auto i = 0; i < axel::CONTEXT->playerCount; i++) {
						if (i != axel::CONTEXT->myAxelId) {
							logger::log_format("[ArcadeManager::NotWaiting] Sending start time to Player: {}...", i);
							auto startGame = axel::message::StartGamePacket::make(axel::CONTEXT->myAxelId, i, 1000);
							std::vector<std::uint8_t> packetBytes(sizeof(axel::message::StartGamePacket));
							std::memcpy(packetBytes.data(), &startGame, sizeof(axel::message::StartGamePacket));
							axel::CONTEXT->network->send_routed_message_async(axel::CONTEXT->lobbyMembers[i].steamId, packetBytes);
						}
					}

					return false;
				}
				else {
					// If we've hit this block, then all opponents are ready, and we have already told them the start time.
					// So we wait until that agreed-upon start time.

					// Once this condition becomes true, we're ready to roll.
					return std::chrono::utc_clock::now() >= axel::CONTEXT->startTime;
				}
			}
			else {
				// If the host hasn't given us a start time yet, we gotta wait it out. If we have been given one, we need to
				// wait until that agreed-upon time.
				if (!axel::CONTEXT->hostRequestedGameStart) {
					return false;
				}
				else {
					return std::chrono::utc_clock::now() >= axel::CONTEXT->startTime;
				}
			}
		}
	}
};

DefineReplacementHook(ShufflePlayerControllersDown) {
	static void __fastcall callback(CarsFrontEnd * _this) {
		original(_this);

		GameEnv_SetWorldData(*g_GameEnv, "DefaultAvatar", "McQueen", *reinterpret_cast<int*>(*g_GameEnv + 0x6180));
		GameEnv_SetWorldData(*g_GameEnv, "DefaultAvatar2", "McQueen", *reinterpret_cast<int*>(*g_GameEnv + 0x6180));
		GameEnv_SetWorldData(*g_GameEnv, "DefaultAvatar3", "McQueen", *reinterpret_cast<int*>(*g_GameEnv + 0x6180));
		GameEnv_SetWorldData(*g_GameEnv, "DefaultAvatar4", "McQueen", *reinterpret_cast<int*>(*g_GameEnv + 0x6180));
		GameEnv_SetWorldData(*g_GameEnv, "DefaultAvatar5", "McQueen", *reinterpret_cast<int*>(*g_GameEnv + 0x6180));
		GameEnv_SetWorldData(*g_GameEnv, "DefaultAvatar6", "McQueen", *reinterpret_cast<int*>(*g_GameEnv + 0x6180));
		GameEnv_SetWorldData(*g_GameEnv, "DefaultAvatar7", "McQueen", *reinterpret_cast<int*>(*g_GameEnv + 0x6180));
		GameEnv_SetWorldData(*g_GameEnv, "DefaultAvatar8", "McQueen", *reinterpret_cast<int*>(*g_GameEnv + 0x6180));
		GameEnv_SetWorldData(*g_GameEnv, "DefaultAvatar9", "McQueen", *reinterpret_cast<int*>(*g_GameEnv + 0x6180));
		GameEnv_SetWorldData(*g_GameEnv, "DefaultAvatar10", "McQueen", *reinterpret_cast<int*>(*g_GameEnv + 0x6180));
	}
};

const char* PLAYER_NAMES[10] = {
	"Player1",
	"Player2",
	"Player3",
	"Player4",
	"Player5",
	"Player6",
	"Player7",
	"Player8",
	"Player9",
	"Player10",
};

static bool sentFirstPacket;
static std::chrono::system_clock::time_point previousPacketSendTime;

auto send_vehicle_state() -> void {
	axel::VehicleState& myState = axel::CONTEXT->vehicleStates[axel::CONTEXT->myAxelId];

	if (!sentFirstPacket || (std::chrono::system_clock::now() - previousPacketSendTime) >= axel::TICK_TIME) {
		for (int i = 0; i < axel::player_count(); i++) {
			if (i != axel::CONTEXT->myAxelId) {
				auto header = axel::message::VehicleStatePacket::make(axel::CONTEXT->myAxelId, i, myState);
				std::vector<std::uint8_t> packetBytes(sizeof(axel::message::VehicleStatePacket));
				std::memcpy(packetBytes.data(), &header, sizeof(axel::message::VehicleStatePacket));
				axel::CONTEXT->network->send_routed_message_async(axel::CONTEXT->lobbyMembers[i].steamId, packetBytes);
			}
		}
		previousPacketSendTime = std::chrono::system_clock::now();
		sentFirstPacket = true;
	}
}

static float PREVIOUS_RACE_TIME = 0.0f;

DefineReplacementHook(UpdateAIMgr) {
	static void __fastcall callback(void* _this, std::uintptr_t edx, void* input, void* output) {
		if (!axel::online()) {
			return;
		}

		float raceTime = (*g_RaceManager)->GetRaceTime(0);
		if (raceTime == 0.0f) {
			return;
		}

		// Time passed since *our* last call to this function. (Not to be confused with the latency compensation dt!)
		double localDt = raceTime - PREVIOUS_RACE_TIME;
		
		int actorCount = *reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(_this) + 0x10);
		for (int i = 0; i < (std::min)(actorCount, 10); i++) {
			if (i >= axel::player_count())
				continue;

			void* aiPlayer = DrivingAI_cAIManager_GetPlayerPtr(_this, i);
			ActiveMoves* activeMoves = *reinterpret_cast<ActiveMoves**>(reinterpret_cast<std::uintptr_t>(aiPlayer) + 0x7C);
			CarsVehicle* vehicle = *reinterpret_cast<CarsVehicle**>(reinterpret_cast<std::uintptr_t>(aiPlayer) + 0x80);
			CarsReactionMonitor* reactionMonitor = *reinterpret_cast<CarsReactionMonitor**>(reinterpret_cast<std::uintptr_t>(aiPlayer) + 0x84);
			if (i == 0) {
				axel::VehicleState& myState = axel::CONTEXT->vehicleStates[axel::CONTEXT->myAxelId];

				void** input = DrivingAI_cAIManagerInput_GetPlayerInput(reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(_this) + 4), i);
				Vector3 position = *reinterpret_cast<Vector3*>(reinterpret_cast<std::uintptr_t>(*input) + 0x28);
				Vector3 velocity = *reinterpret_cast<Vector3*>(reinterpret_cast<std::uintptr_t>(*input) + 0x58);
				Matrix3x3 orientation = *reinterpret_cast<Matrix3x3*>(reinterpret_cast<std::uintptr_t>(*input) + 0x34);

				myState.actionState = std::to_underlying(activeMoves->m_actionState);
				myState.jumpState = std::to_underlying(activeMoves->m_jumpState);
				myState.jumpTrickState = std::to_underlying(activeMoves->m_jumpTrickState);
				myState.driftWallRideState = std::to_underlying(activeMoves->m_driftWallRideState);
				myState.tauntState = std::to_underlying(activeMoves->m_tauntState);
				myState.isTurboing = activeMoves->m_turboing;
				myState.raceTime = raceTime;

				myState.position = position;
				myState.velocity = velocity;
				myState.orientation = orientation;
				myState.acceleration = (localDt <= 1E-5) ? Vector3() : (myState.velocity - axel::CONTEXT->myPreviousState.velocity) * (1.0 / localDt);

				send_vehicle_state();

				axel::CONTEXT->myPreviousState = myState;
			}
			else {
				int opponentAxelId = axel::to_axel_id(i);
				// Negative latency is bad!
				double dt = (raceTime - axel::CONTEXT->vehicleStates[opponentAxelId].raceTime);
				if (dt < 0) {
					dt = 0;
				}

				Vector3 dx = 0.5 * axel::CONTEXT->vehicleStates[opponentAxelId].acceleration * dt * dt;
				dx += (axel::CONTEXT->vehicleStates[opponentAxelId].velocity * dt);
				Vector3 position = axel::CONTEXT->vehicleStates[opponentAxelId].position + dx;

				logger::log_format("[axel::Update] Latency: {}, VelComp: {}, AccelComp: {}", dt, (axel::CONTEXT->vehicleStates[opponentAxelId].velocity * dt).Length(), (0.5 * axel::CONTEXT->vehicleStates[opponentAxelId].acceleration * dt * dt).Length());

				// Vector3 position = axel::CONTEXT->vehicleStates[opponentAxelId].position;
				Vector3 velocity = axel::CONTEXT->vehicleStates[opponentAxelId].velocity;
				Matrix3x3 orient = axel::CONTEXT->vehicleStates[opponentAxelId].orientation;
				Matrix4x4 transform = Matrix4x4::FromRotTrans(&orient, &position);

				// Apply ActiveMoves state.
				ActiveMoves::ActionState opponentState = static_cast<ActiveMoves::ActionState>(axel::CONTEXT->vehicleStates[opponentAxelId].actionState);
				if (opponentState == ActiveMoves::ActionState::SideStepLeft) {
					activeMoves->SetSideSteppingLeft(true);
				}
				else if (opponentState == ActiveMoves::ActionState::SideStepRight) {
					activeMoves->SetSideSteppingRight(true);
				}
				else if (opponentState == ActiveMoves::ActionState::TwoWheelLeft) {
					activeMoves->SetTwoWheelingLeft(true);
				}
				else if (opponentState == ActiveMoves::ActionState::TwoWheelRight) {
					activeMoves->SetTwoWheelingRight(true);
				}
				else if (opponentState == ActiveMoves::ActionState::DriftLeft) {
					activeMoves->SetDriftingLeft(true, false);
				}
				else if (opponentState == ActiveMoves::ActionState::DriftRight) {
					activeMoves->SetDriftingRight(true, false);
				}
				else if (opponentState == ActiveMoves::ActionState::BackwardsDriving) {
					activeMoves->SetBackwardsDriving(true);
				}
				else if (opponentState == ActiveMoves::ActionState::BunnyHop) {
					activeMoves->SetBunnyHopping(true, false);
				}

				activeMoves->m_turboing = axel::CONTEXT->vehicleStates[opponentAxelId].isTurboing;

				Physics_PhysicsMgr_TeleportRigidBody((*g_CollisionSystem) + 0x11D4, *reinterpret_cast<void**>(reinterpret_cast<std::uintptr_t>(vehicle) + 0x9CC), &transform, 1);
			}
		}

		PREVIOUS_RACE_TIME = raceTime;
	}
};

DefineReplacementHook(GetMaxPC) {
	static int _cdecl callback() {
		if (axel::online()) {
			return 10;
		}
		return 4;
	}
};

#pragma region Player_Expansion

DefineReplacementHook(CarsHudGetGuage) {
	static HudGauge* _fastcall callback(CarsHud* _this, std::uintptr_t edx, std::int8_t index) {
		if (index < 0 || index > 3) {
			return nullptr;
		}
		if (_this->hud[index] == nullptr) {
			return nullptr;
		}
		return _this->hud[index]->m_hudGuage;
	}
};

DefineReplacementHook(CarsHudGetHudPosition) {
	static HudPosition* _fastcall callback(CarsHud * _this, std::uintptr_t edx, std::int8_t index) {
		if (index < 0 || index > 3) {
			return nullptr;
		}
		GameProgressionManager::MissionMode mode = (*g_GameProgressionManager)->GetMissionMode();
		if (mode == GameProgressionManager::MissionMode::Collect || mode == GameProgressionManager::MissionMode::Hunter || mode == GameProgressionManager::MissionMode::Tutorial) {
			if (_this->m_hudPosition != nullptr) {
				return _this->m_hudPosition;
			}
		}
		if (_this->hud[index] == nullptr) {
			return nullptr;
		}
		return _this->hud[index]->m_hudPosition;
	}
};

DefineReplacementHook(CarsHudGetRaceClocks) {
	static HudClocks* _fastcall callback(CarsHud * _this, std::uintptr_t edx, std::int8_t index) {
		if (index < 0 || index > 3) {
			return nullptr;
		}
		if (_this->hud[index] == nullptr) {
			return nullptr;
		}
		return _this->hud[index]->m_hudClocks;
	}
};

DefineReplacementHook(CarsHudGetMPHud) {
	static PlayerHud* _fastcall callback(CarsHud * _this, std::uintptr_t edx, std::int8_t index) {
		if (index < 0 || index > 3) {
			return nullptr;
		}
		if (_this->hud[index] == nullptr) {
			return nullptr;
		}
		return _this->hud[index];
	}
};

DefineReplacementHook(CarsHudGetActiveInfo) {
	static HudActiveInfo* _fastcall callback(CarsHud * _this, std::uintptr_t edx, std::int8_t index) {
		if (index < 0 || index > 3) {
			return nullptr;
		}
		if (_this->hud[index] == nullptr) {
			return nullptr;
		}
		return _this->hud[index]->m_hudActiveInfo;
	}
};

DefineReplacementHook(CarsHudGetSkateReticule) {
	static HudSkateReticule* _fastcall callback(CarsHud * _this, std::uintptr_t edx, std::int8_t index) {
		if (index < 0 || index > 3) {
			return nullptr;
		}
		if (_this->hud[index] == nullptr) {
			return nullptr;
		}
		return _this->hud[index]->m_hudSkateReticule;
	}
};

DefineReplacementHook(CarsHudGetTrickCallout) {
	static HudTrickCallout* _fastcall callback(CarsHud * _this, std::uintptr_t edx, std::int8_t index) {
		if (index < 0 || index > 3) {
			return nullptr;
		}
		if (_this->hud[index] == nullptr) {
			return nullptr;
		}
		return _this->hud[index]->m_hudTrickCallout;
	}
};

DefineReplacementHook(CarsHudGetHealthBar) {
	static HudHealthBar* _fastcall callback(CarsHud * _this, std::uintptr_t edx, std::int8_t index) {
		if (index < 0 || index > 3) {
			return nullptr;
		}
		if (_this->hud[index] == nullptr) {
			return nullptr;
		}
		return _this->hud[index]->m_hudHealthBar;
	}
};

DefineReplacementHook(CarsHudGetPlace) {
	static HudPlace* _fastcall callback(CarsHud * _this, std::uintptr_t edx, std::int8_t index) {
		if (index < 0 || index > 3) {
			return nullptr;
		}
		if (_this->hud[index] == nullptr) {
			return nullptr;
		}
		return _this->hud[index]->m_hudPlace;
	}
};

DefineReplacementHook(CarsHudGetScore) {
	static HudScore* _fastcall callback(CarsHud * _this, std::uintptr_t edx, std::int8_t index) {
		if (index < 0 || index > 3) {
			return nullptr;
		}
		if (_this->hud[index] == nullptr) {
			return nullptr;
		}
		return _this->hud[index]->m_hudScore;
	}
};

DefineReplacementHook(CarsHudGetFrame) {
	static HudFrame* _fastcall callback(CarsHud * _this, std::uintptr_t edx, std::int8_t index) {
		if (index < 0 || index > 3) {
			return nullptr;
		}
		if (_this->hud[index] == nullptr) {
			return nullptr;
		}
		return _this->hud[index]->m_hudFrame;
	}
};

DefineReplacementHook(CarsHudGetTimer) {
	static HudTimer* _fastcall callback(CarsHud * _this, std::uintptr_t edx, std::int8_t index) {
		GameProgressionManager::MissionMode mode = (*g_GameProgressionManager)->GetMissionMode();
		if (mode == GameProgressionManager::MissionMode::Takedown) {
			if (index < 0 || index > 3) {
				return nullptr;
			}
			else if (_this->hud[index] == nullptr || 1 < _this->m_playerCount) {
				return _this->m_hudTimer;
			}
			else {
				return _this->hud[index]->m_hudTimer;
			}
		}
		if (mode == GameProgressionManager::MissionMode::Hunter) {
			if (index < 0 || index > 3) {
				return nullptr;
			}
			else if (_this->hud[index] == nullptr || _this->m_playerCount != 1) {
				return _this->m_hudTimer;
			}
			else {
				return _this->hud[index]->m_hudTimer;
			}
		}
		return _this->m_hudTimer;
	}
};

DefineReplacementHook(CarsHudGetTeamMarkers) {
	static HudTeamMarkers* _fastcall callback(CarsHud * _this, std::uintptr_t edx, std::int8_t index) {
		if (index < 0 || index > 3) {
			return nullptr;
		}
		if (_this->hud[index] == nullptr) {
			return nullptr;
		}
		return _this->hud[index]->m_hudTeamMarkers;
	}
};

DefineReplacementHook(CarsHudGet1Up) {
	static Hud1Up* _fastcall callback(CarsHud * _this, std::uintptr_t edx, std::int8_t index) {
		if (index < 0 || index > 3) {
			return nullptr;
		}
		if (_this->hud[index] == nullptr) {
			return nullptr;
		}
		return _this->hud[index]->m_hud1Up;
	}
};

DefineReplacementHook(CarsHudGetMissionMeter) {
	static HudMissionMeter* _fastcall callback(CarsHud * _this, std::uintptr_t edx, std::int8_t index) {
		if (index < 0 || index > 3) {
			return nullptr;
		}
		if (_this->hud[index] == nullptr) {
			return nullptr;
		}
		return _this->hud[index]->m_hudMissionMeter;
	}
};

/*
DefineReplacementHook(CarsHudGetTutorialButtonPrompt) {
	static HudTutorialButtonPrompt* _fastcall callback(CarsHud * _this, std::uintptr_t edx, std::int8_t index) {
		if (index < 0 || index > 3) {
			return nullptr;
		}
		if (_this->hud[index] == nullptr) {
			return nullptr;
		}
		return _this->hud[index]->m_hudTutorialButtonPrompt;
	}
};
*/
auto expand_dong() {
	// `CarsHud::GetPlayerIndexFromMovie` oddly enough, checks if the index is > 10, instead of 4; That's very suspicious!
	sunset::utils::set_permission(reinterpret_cast<void*>(0x00551c7b + 3), sizeof(char), sunset::utils::Perm::ExecuteReadWrite);
	*reinterpret_cast<char*>(0x00551c7b + 3) = 5;

	// Adds more bounds-checks to `CarsHud` functions.
	CarsHudGetGuage::install_at_ptr(0x00552960);
	CarsHudGetHudPosition::install_at_ptr(0x005529a0);
	CarsHudGetRaceClocks::install_at_ptr(0x00552a50);
	CarsHudGetMPHud::install_at_ptr(0x00552b50);
	CarsHudGetActiveInfo::install_at_ptr(0x00552b80);
	CarsHudGetSkateReticule::install_at_ptr(0x00552bc0);
	CarsHudGetTrickCallout::install_at_ptr(0x00552c00);
	CarsHudGetHealthBar::install_at_ptr(0x00552cf0);
	CarsHudGetPlace::install_at_ptr(0x00552d30);
	CarsHudGetScore::install_at_ptr(0x00552d70);
	CarsHudGetFrame::install_at_ptr(0x00552db0);
	CarsHudGetTimer::install_at_ptr(0x00552df0);
	CarsHudGetTeamMarkers::install_at_ptr(0x00552ef0);
	CarsHudGet1Up::install_at_ptr(0x00552f30);
	CarsHudGetMissionMeter::install_at_ptr(0x00552f70);
}

#pragma endregion

DefineInlineHook(PlayerSuspensionInit) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		ctx.edx.unsigned_integer = std::clamp(ctx.edx.unsigned_integer, 1u, 4u);
	}
};

auto axel::ingame::install_hooks() -> void {
	GetStartingGridLocForPlayer::install_at_ptr(0x004ea100);
	SetStartingCarId::install_at_ptr(0x004ebc60);
	FillStartingGrid::install_at_ptr(0x004e9230);
	ArcadeManager_CanStart::install_at_ptr(0x004591d0);
	ShufflePlayerControllersDown::install_at_ptr(0x004c4fa0);

	UpdateAIMgr::install_at_ptr(0x004089f0);

	sunset::utils::set_permission(reinterpret_cast<void*>(0x00ec2a29 + 3), sizeof(std::uint32_t), sunset::utils::Perm::ExecuteReadWrite);
	*reinterpret_cast<std::uint32_t*>(0x00ec2a29 + 3) = reinterpret_cast<std::uintptr_t>(PLAYER_NAMES);

	GetMaxPC::install_at_ptr(0x00554010);

	// Player expansion dick:
	// sunset::inst::nop(reinterpret_cast<void*>(0x004f2840), 5);
	// Meridian::PlayerSuspsensionEventNode Hack
	// PlayerSuspensionInit::install_at_ptr(0x0058fb63);
	// expand_dong();
	// Player expansion dick end!
}