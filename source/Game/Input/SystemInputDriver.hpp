#pragma once
#include "../../Patch/Input/ControllerInputDriver.hpp"

enum ButtonCode {
    BC_A,
    BC_B,
    BC_C,
    BC_D,
    BC_E,
    BC_F,
    BC_G,
    BC_H,
    BC_I,
    BC_J,
    BC_K,
    BC_L,
    BC_M,
    BC_N,
    BC_O,
    BC_P,
    BC_Q,
    BC_R,
    BC_S,
    BC_T,
    BC_U,
    BC_V,
    BC_W,
    BC_X,
    BC_Y,
    BC_Z,
    BC_0,
    BC_1,
    BC_2,
    BC_3,
    BC_4,
    BC_5,
    BC_6,
    BC_7,
    BC_8,
    BC_9,
    BC_SPACE,
    BC_TILDE,
    BC_MINUS,
    BC_PLUS,
    BC_LEFTBRACKET,
    BC_RIGHTBRACKET,
    BC_FORWARDSLASH,
    BC_SEMICOLON,
    BC_QUOTE,
    BC_PERIOD,
    BC_COMMA,
    BC_BACKSLASH,
    BC_ESCAPE,
    BC_BACKSPACE,
    BC_TAB,
    BC_ENTER,
    BC_INSERT,
    BC_DELETE,
    BC_HOME,
    BC_END,
    BC_PAGEUP,
    BC_PAGEDOWN,
    BC_UP,
    BC_DOWN,
    BC_LEFT,
    BC_CENTER,
    BC_RIGHT,
    BC_MULTIPLY,
    BC_ADD,
    BC_SUBTRACT,
    BC_DIVIDE,
    BC_NUM_0,
    BC_NUM_1,
    BC_NUM_2,
    BC_NUM_3,
    BC_NUM_4,
    BC_NUM_5,
    BC_NUM_6,
    BC_NUM_7,
    BC_NUM_8,
    BC_NUM_9,
    BC_NUM_DOT,
    BC_NUM_ENTER,
    BC_NUM_LOCK,
    BC_F1,
    BC_F2,
    BC_F3,
    BC_F4,
    BC_F5,
    BC_F6,
    BC_F7,
    BC_F8,
    BC_F9,
    BC_F10,
    BC_F11,
    BC_F12,
    BC_MOUSELEFT,
    BC_MOUSEMIDDLE,
    BC_MOUSERIGHT,
    BC_MOUSEWHEELUP,
    BC_MOUSEWHEELDOWN,
    BC_LSHIFT,
    BC_LCONTROL,
    BC_LALT,
    BC_RSHIFT,
    BC_RCONTROL,
    BC_RALT,
    BC_INVALID,
    BC_BUTTONCODES
};

class SystemInputDriver {
public:
	ButtonState state[104];
	int mouseX;
	int mouseY;
	bool hasFocus;
	bool keyboardEnabled;
public:
	SystemInputDriver();
	SystemInputDriver(const SystemInputDriver&) = delete;
	SystemInputDriver& operator=(const SystemInputDriver&) = delete;

	virtual ~SystemInputDriver();
	virtual void BeginInput();
	virtual void ClearInput();
	virtual void DoneInput();
	virtual bool HardwareResetPressed();
	virtual bool HardwarePowerDown();
	virtual bool KeyboardConnected();
	virtual bool ButtonPressed(ButtonCode button);
	virtual void ClearButtonDown(ButtonCode button);
	virtual bool ButtonReleased(ButtonCode button);
	virtual bool ButtonDown(ButtonCode button);
	virtual bool MouseConnected();
	virtual void Unk();
	virtual void Unk2();
	virtual void* GetRawControllerData(int port) const;
	virtual unsigned int Controllers() = 0;
	virtual ControllerInputDriver* GetController(unsigned int player) = 0;
	virtual void UnlockAllControllers();
	virtual void LockPlayerToController(unsigned int player, int controllerNum);
	virtual void UnlockPlayerFromController(unsigned int playerNum);
	virtual bool ControllerLocked(unsigned int player);
	virtual bool IsControllerLocked(unsigned int controllerIndex);
	virtual bool UnmappedControllerLocked(unsigned int controllerNum);
	virtual int GetLockedControllerIndex(unsigned int player);
	virtual int GetFirstUnlockedControllerIndexStartingFrom(unsigned int startIndex);
	virtual int GetFirstUnlockedControllerIndex();
	virtual ControllerInputDriver* GetUnlockedController(unsigned int player);
	virtual ControllerInputDriver* GetAssertController();
	virtual ControllerInputDriver* GetFirstUnlockedController();
	virtual unsigned int GetNumLockedControllers();
	virtual int GetFirstUnlockedPlayer();
	virtual int GetPlayerIndex(ControllerInputDriver* controller);
};

static_assert(sizeof(SystemInputDriver) == 224);