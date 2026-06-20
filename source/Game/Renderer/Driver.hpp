#pragma once
#include <Windows.h>
#include <d3d9.h>
#include "../Types.hpp"

inline auto Renderer_DriverImpl_PumpMessages = (bool(__thiscall*)(void*))(0x00832990);

namespace Renderer {
	
    class Material;
	class SceneNode;

    class DriverConfiguration : public Types::ReferenceCountable {
    public:
        HINSTANCE m_instance;
        WNDPROC m_windowProc;
        bool m_fullscreen;
        int m_windowX;
        int m_windowY;
        unsigned int m_windowWidth;
        unsigned int m_windowHeight;
        HWND m_parentWindow;
        HICON m_icon;
        HCURSOR m_cursor;
        const char* m_windowClassName;
    };

	class DriverImpl {
	public:
        bool m_enabled;
        HWND m_window;
        HANDLE m_thread;
        struct IDirect3D9Ex* m_d3d;
        bool m_isWideScreen;
        DriverConfiguration m_configuration;
        bool m_maximized;
        bool m_minimized;
        bool m_windowed;
        int m_fullScreenResolutionID;
        int m_keepAspectRatio;
        bool m_resizeRequested;
        unsigned int m_lastWin32Error;
        int m_currentAdapter;
        D3DDEVTYPE m_deviceType;
        unsigned int m_windowState;
        int m_windowStyleAdjustTop;
        int m_windowStyleAdjustLeft;
        int m_windowStyleAdjustWidth;
        int m_windowStyleAdjustHeight;
        unsigned int m_threadId;
        D3DPRESENT_PARAMETERS m_presentParametersW;
        D3DPRESENT_PARAMETERS m_presentParametersF;
        D3DDISPLAYMODEEX m_fullScreenDisplayModeEx;
        bool m_usingD3D9EX;
        char m_dxDataBase[100];
        class Driver* m_driver;
        unsigned int m_presentationInterval;
        bool m_supportsAnisotropicMagFilter;
        unsigned int m_maxSupportedAnisotropy;

        inline bool PumpMessages() {
            return Renderer_DriverImpl_PumpMessages(this);
        }
	};

	class Driver {
	public:
		Material* m_errorMaterial;
		Material* m_textMaterial;
		Material* m_spriteMaterial;
		DriverImpl* m_impl;
		SceneNode* m_sceneGraph;
		Material* m_sharedMaterial;
		Material* m_unkMaterial;
		void* unused0[16];
		int unused1;
		int unused2[10];
		unsigned int m_captureFormat;
		char* m_captureFileName;
		bool m_exitRequested;
		bool m_initialized;
	};

	inline auto g_Driver = reinterpret_cast<Driver**>(0x01906308);

    static_assert(sizeof(DriverConfiguration) == 0x38);
    static_assert(sizeof(Driver) == 0x94);
    static_assert(sizeof(DriverImpl) == 0x180);
};