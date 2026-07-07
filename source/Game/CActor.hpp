#pragma once
#include <cstdint>
#include "../Game/Stage/StageEntity.hpp"
#include "../Game/Stage/ActivityDBlock.hpp"
#include "../Game/Utils/Vector3.hpp"

inline auto CActor_GetComponentByName = (class CActorComponent*(__thiscall*)(class CActor*, const char*))(0x00e05260);
inline auto CActor_FromHandle = (class CActor*(_cdecl*)(ActorHandle))(0x00e19e20);
inline auto CActor_GetDataBlock = (DBlock* (__thiscall*)(class CActor*, DBlock::BlockType, unsigned int))(0x00e19820);

class CActor : public CStageEntity, public ActivityDBlock {
public:
	char reserved[12];
public:
	inline virtual bool IsUnstableCollisionObject(void) override {
		return true;
	}

	static inline CActor* FromHandle(ActorHandle handle) {
		return CActor_FromHandle(handle);
	}

	inline Vector3 GetBodyInWorld() {
		return *reinterpret_cast<Vector3*>(reinterpret_cast<std::uintptr_t>(this) + 0x58);
	}

	inline ActorHandle GetHandle() {
		return *reinterpret_cast<ActorHandle*>(reinterpret_cast<std::uintptr_t>(this) + 0x24);
	}
	
	inline CActorComponent* GetComponentByName(const char* name) {
		return CActor_GetComponentByName(this, name);
	}

	inline DBlock* GetDataBlock(DBlock::BlockType type, unsigned int id = DBlock::DefaultID) {
		return CActor_GetDataBlock(this, type, id);
	}
};