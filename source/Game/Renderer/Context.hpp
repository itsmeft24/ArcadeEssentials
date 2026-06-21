#pragma once
namespace Renderer {

	/*
	struct Flags {
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
		unsigned int flags;
	};

	struct C2_RenderConfig {
		bool ssaoEnabled;
		bool worldShadowEnabled;
		bool volumetricLightEnabled;
		bool reflectionPlaneEnabled;
		float shadowMultiplier;
		unsigned int shadowCascadeDecrement;
		float shadowDistance;
		bool pip;
		bool halfResTargetEnabled;
		short unknown;
	};

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

	struct R_TargetBuffers {
		int depth;
		int color[4];
		struct R_Target* field2_0x14;
		struct R_Target* field3_0x18[4];
	};


	struct R_Target {
		unsigned int res;
		unsigned char writeIndex;
		unsigned char readIndex;
		unsigned char numBuffers;
		unsigned char cycleBuffersOnResolve;
		struct R_TargetBuffers buffers[2];
		short width;
		short height;
		struct R_Viewport viewport;
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

	struct C2_Scene {
		struct SceneNode* root;
		struct CameraNode* camera;
		R_Viewport viewport;
		R_Target* target;
		unsigned int sceneList;
		void* markers;
		unsigned short numMarkers;
		struct C2_RenderConfig config;
		unsigned short sceneIndex;
		unsigned short sceneCount;
		unsigned int displayLayerMask;
	};

	struct R_Job {
		struct R_Job* next;
		unsigned int dataIn;
		unsigned int datasize;
		void* submitFn;
		void* executeFn;
		struct R_Buffer* buffer;
		struct R_Viewport scissorRegion;
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


	struct SceneRenderContext {
		struct R_Buffer* buffer;
		struct CameraNode* camera;
		struct SceneNode* sceneRoot;
		struct R_Viewport viewport;
		struct R_Target* target;
		unsigned int sceneList;
		struct R_Buffer* params;
		char unknown[32];
		unsigned int translucentTargetId;
		unsigned int sceneIndex;
		int sceneCount;
		struct C2_RenderConfig config;
		struct R_Buffer* textures;
		struct R_Target* targets;
		void* materials;
		unsigned char shadowIndex;
	};


	inline R_Context** r_context = reinterpret_cast<R_Context**>(0x019063b4);
}