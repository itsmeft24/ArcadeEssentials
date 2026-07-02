#pragma once

namespace Renderer {
	using R_SubmitJobsFn = void(_cdecl*)();

	struct R_Target;
	struct R_Buffer;
	
	struct R_Viewport {
		short x;
		short y;
		short w;
		short h;
		float zNear;
		float zFar;
		float aspectX;
		float aspectY;
	};

	/*
	struct ContextFlags {
		unsigned int haveSubmitted : 1; // Confirmed, 0th bit
		unsigned int isMultiThreaded : 1; // Confirmed, 1st bit
		unsigned int inFrontEnd : 1;
		unsigned int displayLoadScreen : 1; // Confirmed, 3rd bit
		unsigned int displayMainScene : 1;
		unsigned int captureScene : 1;
		unsigned int isRendering : 1; // Confirmed, 6th bit
		unsigned int anisoQuality : 2;
	};
	*/

	struct R_Context {
		char unknown[0x120];
		unsigned int flags; // See ContextFlags
	};

	struct R_Job {
		R_Job* next;
		unsigned int dataIn;
		unsigned int datasize;
		void* submitFn;
		void* executeFn;
		R_Buffer* buffer;
		R_Viewport scissorRegion;
		int flags;
		int screenIndex;
		unsigned char field9_0x38;
		unsigned char field10_0x39;
		unsigned char field11_0x3a;
		unsigned char field12_0x3b;
		unsigned char field13_0x3c;
		unsigned char field14_0x3d;
		unsigned char field15_0x3e;
		unsigned char field16_0x3f;
	};

	inline auto r_waitForRenderThread = (void(_cdecl*)())(0x008745b0);
	inline auto r_reset = (void(_cdecl*)())(0x00874440);
	inline auto r_renderFrame = (void(_cdecl*)())(0x008756e0);
	inline auto r_setSubmitJobsFn = (R_SubmitJobsFn(_cdecl*)(R_SubmitJobsFn))(0x00874110);
	inline auto r_defaultTarget = (R_Target*(_cdecl*)())(0x0087bf70);
	inline auto r_cycleTargetBuffers = (void(_cdecl*)(R_Target*))(0x00876260);

	inline R_Context** r_context = reinterpret_cast<R_Context**>(0x019063b4);
};