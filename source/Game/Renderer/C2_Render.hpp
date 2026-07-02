#pragma once
#include "R_Buffer.hpp"
#include "R_Render.hpp"
#include "SceneNodes/CameraNode.hpp"

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

struct C2_Scene {
	Renderer::SceneNode* root;
	Renderer::CameraNode* camera;
	Renderer::R_Viewport viewport;
	Renderer::R_Target* target;
	unsigned int sceneList;
	void* markers;
	unsigned short numMarkers;
	struct C2_RenderConfig config;
	unsigned short sceneIndex;
	unsigned short sceneCount;
	unsigned int displayLayerMask;
};

struct SceneRenderContext {
	Renderer::R_Buffer* buffer;
	Renderer::CameraNode* camera;
	Renderer::SceneNode* sceneRoot;
	Renderer::R_Viewport viewport;
	Renderer::R_Target* target;
	unsigned int sceneList;
	Renderer::R_Buffer* params;
	char unknown[32];
	unsigned int translucentTargetId;
	unsigned int sceneIndex;
	int sceneCount;
	C2_RenderConfig config;
	Renderer::R_Buffer* textures;
	Renderer::R_Target* targets;
	void* materials;
	unsigned char shadowIndex;
};

inline auto	_build_scene_context = (SceneRenderContext * (_cdecl*)(C2_Scene*))(0x0062c380);
inline auto	_submit_scene_bind = (void(_cdecl*)(SceneRenderContext*))(0x0062d6f0);
inline auto	_submit_dynamic_reflect_component = (void(_cdecl*)(SceneRenderContext*))(0x0062dad0);
inline auto	_submit_car_eyes = (void(_cdecl*)(SceneRenderContext*))(0x0062db00);
inline auto	_submit_world_shadow = (void(_cdecl*)(SceneRenderContext*))(0x0062d840);
inline auto	_submit_reflection_plane = (void(_cdecl*)(SceneRenderContext*))(0x0062de20);
inline auto	_submit_depth = (void(_cdecl*)(SceneRenderContext*))(0x0062d860);
inline auto	_submit_downsample_depth = (void(_cdecl*)(SceneRenderContext*))(0x0062da50);
inline auto	_submit_stencil = (void(_cdecl*)(SceneRenderContext*))(0x0062e170);
inline auto	_add_shadow_mask_target_id = (void(_cdecl*)(SceneRenderContext*))(0x0062d940);
inline auto	_submit_shadowMask = (void(_cdecl*)(SceneRenderContext*))(0x0062d920);
inline auto	_submit_shadowApply = (void(_cdecl*)(SceneRenderContext*))(0x0062d560);
inline auto	_submit_main_scene = (void(_cdecl*)(SceneRenderContext*))(0x0062c5a0);
inline auto	_submit_particles = (void(_cdecl*)(SceneRenderContext*))(0x0062d960);
inline auto	_submit_luminance = (void(_cdecl*)(SceneRenderContext*))(0x0062c870);
inline auto	_submit_motion_blur = (void(_cdecl*)(SceneRenderContext*))(0x0062d5e0);
inline auto	_submit_bloom = (void(_cdecl*)(SceneRenderContext*))(0x0062ca90);
inline auto	_submit_post_process = (void(_cdecl*)(SceneRenderContext*))(0x0062d280);
inline auto	g_WorldShadowsEnabled = reinterpret_cast<unsigned int*>(0x01856a84);