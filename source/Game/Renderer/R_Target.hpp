#pragma once
#include "R_Render.hpp"
#include "R_Texture.hpp"

namespace Renderer {
	struct R_Target;
	struct R_Viewport;

	struct R_TargetBuffers {
		R_Texture* depth;
		R_Texture* color[4];
		R_Target* field2_0x14;
		R_Target* field3_0x18[4];
	};

	struct R_Target {
		unsigned int res;
		unsigned char writeIndex;
		unsigned char readIndex;
		unsigned char numBuffers;
		unsigned char cycleBuffersOnResolve;
		R_TargetBuffers buffers[2];
		short width;
		short height;
		R_Viewport viewport;
		char name[32];
		unsigned char field10_0x94;
		unsigned char field11_0x95;
		unsigned char field12_0x96;
		unsigned char field13_0x97;
		unsigned char field14_0x98;
		unsigned char field15_0x99;
		unsigned char field16_0x9a;
		unsigned char field17_0x9b;
		unsigned char field18_0x9c;
		unsigned char field19_0x9d;
		unsigned char field20_0x9e;
		unsigned char field21_0x9f;
		unsigned char field22_0xa0;
		unsigned char field23_0xa1;
		unsigned char field24_0xa2;
		unsigned char field25_0xa3;
		unsigned char field26_0xa4;
		unsigned char field27_0xa5;
		unsigned char field28_0xa6;
		unsigned char field29_0xa7;
		D3DCOLOR clearColor;
		float clearDepth;
		bool useScissorRect;
		RECT scissorRect;
	};
};