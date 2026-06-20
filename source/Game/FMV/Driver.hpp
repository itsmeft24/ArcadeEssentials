#pragma once
#include "Subtitles.hpp"
#include "../DynamicArray.hpp"
#include "../Renderer/Texture.hpp"
#include <d3d9.h>

inline auto FMV_Driver_CreateTextures = (bool(__thiscall*)(void*, void*))(0x00cdb360);

struct BINKFRAMETEXTURES {
	unsigned int field0_0x0;
	unsigned int field1_0x4;
	unsigned int field2_0x8;
	unsigned int field3_0xc;
	IDirect3DTexture9* field4_0x10;
	IDirect3DTexture9* field5_0x14;
	IDirect3DTexture9* field6_0x18;
	IDirect3DTexture9* field7_0x1c;
};

struct BINKPLANE {
	int Allocate;
	void* Buffer;
	unsigned int BufferPitch;
};

struct BINKFRAMEPLANESET {
	BINKPLANE YPlane;
	BINKPLANE cRPlane;
	BINKPLANE cBPlane;
	BINKPLANE APlane;
};

struct BINKFRAMEBUFFERS {
	int TotalFrames;
	unsigned int YABufferWidth;
	unsigned int YABufferHeight;
	unsigned int cRcBBufferWidth;
	unsigned int cRcBBufferHeight;
	unsigned int FrameNum;
	struct BINKFRAMEPLANESET Frames[2];
};

struct BINKTEXTURESET {
	BINKFRAMETEXTURES textures[2];
	BINKFRAMEBUFFERS bink_buffers;
	BINKFRAMETEXTURES tex_draw;
};

struct BINK {
	int Width;
	int Height;
	int Frames;
	int FrameNum;
	char Unknown[40];
	int NumTracks;
};

struct BINKSUMMARY {
	int Width;
	int Height;
	char Unknown[0x74];
};

static_assert(sizeof(BINKSUMMARY) == 0x7C);

namespace FMV {
	class Driver {
	public:
		BINK* m_hBink;
		Types::SmartPointerArray<Renderer::Texture> Y;
		Types::SmartPointerArray<Renderer::Texture> Cb;
		Types::SmartPointerArray<Renderer::Texture> Cr;
		Types::SmartPointerArray<Renderer::Texture> A;
		BINKTEXTURESET m_binkTextureSet;
		Subtitles* m_subTitles;
	public:
		inline bool CreateTextures(BINKTEXTURESET* textureSet) {
			return FMV_Driver_CreateTextures(this, textureSet);
		}
	};
};

static_assert(sizeof(FMV::Driver) == 0x150);