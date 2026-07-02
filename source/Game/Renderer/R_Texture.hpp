#pragma once
#include <d3d9.h>

namespace Renderer {
	struct R_Resource {
		unsigned int type : 8;
		unsigned int refCount : 24;
	};

	struct R_TextureBits {
		unsigned int location : 3;
		unsigned int type : 3;
		unsigned int ownsBuffer : 1;
		unsigned int locked : 1;
		unsigned int mips : 8;
		unsigned int format : 8;
		unsigned int msaaMode : 3;
		unsigned int swizzled : 1;
		unsigned int srgb : 1;
		unsigned int dynamic : 1;
		unsigned int target : 1;
	};

	struct R_Texture {
		R_Resource resource;
		R_TextureBits bits;
		unsigned int size;
		unsigned short width;
		unsigned short height;
		unsigned short depth;
		void* buffer;
		IDirect3DTexture9* d3dTexture;
		IDirect3DSurface9* d3dSurface;
	};
};