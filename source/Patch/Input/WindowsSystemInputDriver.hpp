#pragma once
#include <optional>
#include <dinput.h>
#include "../../Game/Input/SystemInputDriver.hpp"
#include "../../Game/Genie/String.hpp"

inline class WindowsSystemInputDriver** g_InputPtr = reinterpret_cast<class WindowsSystemInputDriver**>(0x018d3244);

constexpr auto MAX_CONTROLLERS = 11;

class WindowsSystemInputDriver : public SystemInputDriver {
public:
	char m_cKeyboardBuffer[256];
	IDirectInput8A* dInput;
	IDirectInputDevice8A* m_pMouse;
	IDirectInputDevice8A* m_pKeyboard;
	IDirectInputDevice8A* m_DevHdl;
	HWND hWnd;
	short keyToButtonMap[256];
	int mapEntries;
	ControllerInputDriver* d_pKeyboard;
	ControllerInputDriver* d_pSpaceBall;
	int controllers;
	ControllerInputDriver* controller[MAX_CONTROLLERS];
	int lockedControllerIndex[MAX_CONTROLLERS];
	int numLockedControllers;
public:
	std::optional<int> FindKBM();
	std::optional<int> FindFirstAvailableControllerSlot();
	void SwapControllers(int i, int j);

	bool Initialize(HINSTANCE hInst, HWND hWnd);
	void BeginInputHook();
};

struct ShowControllerPullPacket {
	Genie::String popupMessage;
	bool unknown;
	int playerIndex;
	int type;
};

struct HideControllerPullPacket {
	int playerIndex;
};

static_assert(sizeof(WindowsSystemInputDriver) == 1120);

bool __fastcall WindowsSystemInputDriver_Initialize(class WindowsSystemInputDriver* _this, std::uintptr_t edx, HINSTANCE hInst, HWND hWnd);
void __fastcall WindowsSystemInputDriver_BeginInput(class WindowsSystemInputDriver* _this);
int __stdcall WindowsSystemInputDriver_EnumJoysticks(const DIDEVICEINSTANCEA* devInst, void* udata);