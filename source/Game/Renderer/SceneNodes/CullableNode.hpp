#pragma once
#include "SceneNode.hpp"
#include "../DynamicArray.hpp"

namespace Renderer {
	struct CullableNode : public SceneNode {
		unsigned int m_Flags;
		unsigned int m_VisObject;
		void* m_SceneRoot;
		DynamicArray<void*> m_ContainingCells;
		void* m_BoundingVolume;
		unsigned char unknown[20];
	};

	static_assert(sizeof(CullableNode) == 0x78);
};