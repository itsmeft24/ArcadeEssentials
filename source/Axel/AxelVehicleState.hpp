#pragma once
#include <cstdint>
#include "../Game/Utils/Vector3.hpp"
#include "../Game/Utils/Matrix.hpp"

namespace axel {
	struct VehicleState {
		Vector3 position = {};
		Vector3 velocity = {};
		Vector3 acceleration = {};
		Matrix3x3 orientation = {};

		int actionState = 0;
		int jumpState = 0;
		int jumpTrickState = 0;
		int driftWallRideState = 0;
		int tauntState = 0;

		int currentWeapon = 0;

		bool isTurboing = false;
		bool isFiring = false;

		double raceTime = 0.0;
	};
};