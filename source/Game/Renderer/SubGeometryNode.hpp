#pragma once
#include <d3d9.h>
#include "CullableNode.hpp"

namespace Renderer {

	struct SubGeometryNodeImpl {
		IDirect3DIndexBuffer9* indexBuffer;
		IDirect3DVertexBuffer9* vertexBuffers[2];
		unsigned int field2_0xc;
		unsigned int field3_0x10;
		unsigned int baseVertexIndex;
		unsigned int streamWidth[2];
		unsigned int streamOffsetInBytes[2];
		unsigned int field7_0x28;
		unsigned int field8_0x2c;
		unsigned int field9_0x30;
		unsigned int field10_0x34;
		unsigned int primCount;
		unsigned int startIndex;
		unsigned int numVertices;
		unsigned int primitiveType;
		DWORD cullMode;
	};
	/*
	struct SubGeometryNode : CullableNode {
		struct CullableNode base;
		undefined field1_0x78;
		undefined field2_0x79;
		undefined field3_0x7a;
		undefined field4_0x7b;
		unsigned int field5_0x7c;
		SubGeometryNodeImpl m_Impl;
		byte m_NumStreams;
		undefined1 field8_0xcd;
		undefined field9_0xce;
		undefined field10_0xcf;
		struct Material* m_material;
		void* m_instanceBlock;
		undefined4 m_renderCaps;
		undefined4 field14_0xdc;
		undefined4 field15_0xe0;
		undefined4 field16_0xe4;
		undefined4 field17_0xe8;
		undefined4 field18_0xec;
		struct IndexStream* m_indexStream;
		struct VertexStream** m_Streams;
		undefined1 field21_0xf8;
		undefined1 field22_0xf9;
		undefined field23_0xfa;
		undefined field24_0xfb;
		undefined4 field25_0xfc;
		undefined4 field26_0x100;
		undefined4 field27_0x104;
		undefined4 field28_0x108;
		struct BoneSetComponent* field29_0x10c;
		struct Matrix4x4* m_BoneMatrices;
		unsigned int m_BoneCount;
	};*/

};