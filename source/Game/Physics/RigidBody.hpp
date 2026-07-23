#pragma once
#include "../../Game/Utils/Matrix.hpp"
#include "../../Game/Utils/Vector3.hpp"
namespace Physics {
	class RigidBody;
};

inline auto Physics_RigidBody_GetPosition = (Vector3*(__thiscall*)(Physics::RigidBody*, Vector3*))(0x00de5510);
inline auto Physics_RigidBody_GetVelocity = (Vector3*(__thiscall*)(Physics::RigidBody*, Vector3*))(0x00de3dc0);
inline auto Physics_RigidBody_GetLocalAngularVelocity = (Vector3*(__thiscall*)(Physics::RigidBody*, Vector3*))(0x00de3ec0);
inline auto Physics_RigidBody_SetVelocity = (void(__thiscall*)(Physics::RigidBody*, const Vector3*))(0x00de3e00);
inline auto Physics_RigidBody_SetLocalAngularVelocity = (void(__thiscall*)(Physics::RigidBody*, const Vector3*))(0x00de3f00);
inline auto Physics_RigidBody_ClearForces = (void(__thiscall*)(Physics::RigidBody*))(0x00de3fc0);
inline auto Physics_RigidBody_ClearDeltas = (void(__thiscall*)(Physics::RigidBody*))(0x00de3fe0);

namespace Physics {
	struct RigidBodyProps {
		Vector3 halfSize;
		float density;
		float mass;
		float invMass;
		Matrix3x3 inertiaTensor;
		Matrix3x3 invInertiaTensor;
		float radius;
		float coeffOfRestitution;
		float groundImpulseRatio;
	};
	struct RigidBodyState {
		Vector3 pos;
		Matrix3x3 orientation;
		Vector3 velocity;
		Vector3 angularMomentum;
		Vector3 force;
		Vector3 torque;
		Matrix3x3 invWorldInertiaTensor;
		Vector3 angularVelocity;
		bool atRest;
	};
	class RigidBody {
	public:
		/*
		Matrix4x4* destination;
		RigidBodyProps* props;
		Vector3 externalForce;
		Vector3 externalTorque;
		RigidBodyState state;
		*/
	public:
		inline virtual ~RigidBody() {
		} 
		inline Vector3 GetPosition() {
			Vector3 out{};
			Physics_RigidBody_GetPosition(this, &out);
			return out;
		}
		inline Vector3 GetVelocity() {
			Vector3 out{};
			Physics_RigidBody_GetVelocity(this, &out);
			return out;
		}
		inline Vector3 GetLocalAngularVelocity() {
			Vector3 out{};
			Physics_RigidBody_GetLocalAngularVelocity(this, &out);
			return out;
		}
		inline void SetVelocity(const Vector3& vec) {
			Physics_RigidBody_SetVelocity(this, &vec);
		}
		inline void SetLocalAngularVelocity(const Vector3& vec) {
			Physics_RigidBody_SetLocalAngularVelocity(this, &vec);
		}
		inline void ClearForces() {
			Physics_RigidBody_ClearForces(this);
		}
		inline void ClearDeltas() {
			Physics_RigidBody_ClearDeltas(this);
		}
	};
};