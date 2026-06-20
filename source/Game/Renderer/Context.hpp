#pragma once
namespace Renderer {

	/*
	struct Flags {
		unsigned int haveSubmitted : 1; // Confirmed, 0th bit
		unsigned int isMultiThreaded : 1; // Confirmed, 1st bit
		unsigned int inFrontEnd : 1;
		unsigned int displayLoadScreen : 1; // Confirmed, 3rd bit
		// unsigned int displayZoneLoadScreen : 1;
		unsigned int displayMainScene : 1;
		unsigned int captureScene : 1;
		unsigned int isRendering : 1; // Confirmed, 6th bit
		unsigned int anisoQuality : 2;
	};
	*/
	
	struct R_Context {
		char unknown[0x120];
		unsigned int flags;
	};

	inline R_Context** r_context = reinterpret_cast<R_Context**>(0x019063b4);
}