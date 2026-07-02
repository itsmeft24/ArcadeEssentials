#pragma once
#include "../Types.hpp"

namespace Renderer {
	struct SceneNode : public Types::GarbageCollectible {
		void* m_instanceClassType;
		unsigned int m_ChildCount;
		struct SceneNode* m_Child;
		struct SceneNode* m_SiblingNext;
		struct SceneNode* m_SiblingPrev;
		char* m_Name;
		unsigned int field7_0x24;
		void* field8_0x28;
		void* field9_0x2c;
		unsigned int field10_0x30;
		struct Types::SmartHandleIndirection* field11_0x34;
	};

	static_assert(sizeof(SceneNode) == 0x38);
};