#pragma once

class DBlock {
public:
	static constexpr unsigned int DefaultID = -1;
	static constexpr unsigned int Flag_MarkedForDeath = 1;

	enum class BlockType : unsigned short {
		Activities = 0,
		ActorOutline = 1,
		Animation = 2,
		Collision = 3,
		Components = 4,
		EntityInstance = 5,
		Geometry = 6,
		Identification = 7,
		Initialization = 8,
		Light = 9,
		Net = 10,
		PointerCache = 11,
		Properties = 12,
		Recording = 13,
		Script = 14,
		Sound = 15,
		Streaming = 16,
		TextureAnim = 17,
		WorldState = 18,
		Cars2Vehicle = 19,
		Num_BlockTypes = 20,
		Custom = 21,
	};
public:
	class CActor* actor;
	BlockType block_ype;
	unsigned short flags;
	unsigned int id;

	inline bool IsMarkedForDeath(void) const {
		return (flags & Flag_MarkedForDeath) == 1;
	}
	inline void MarkForDeath(void) {
		flags |= Flag_MarkedForDeath;
	}
	inline virtual ~DBlock() {
	}
};