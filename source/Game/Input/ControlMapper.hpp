#pragma once
#include "SystemInputDriver.hpp"

inline auto ControlMapper_Destructor = (void(__thiscall*)(class ControlMapper*))(0x0080eb90);
inline auto ControlMapper_Initialize = (void(__thiscall*)(class ControlMapper*))(0x0080ebd0);
inline auto ControlMapper_Update = (void(__thiscall*)(class ControlMapper*))(0x0080ec50);
inline auto ControlMapper_GetAxis = (float(__thiscall*)(const class ControlMapper*, int))(0x0080ef80);
inline auto ControlMapper_ButtonPressed = (bool(__thiscall*)(class ControlMapper*, int, bool))(0x0080f090);
inline auto ControlMapper_ButtonReleased = (bool(__thiscall*)(class ControlMapper*, int, bool))(0x0080f1a0);
inline auto ControlMapper_ButtonDown = (bool(__thiscall*)(class ControlMapper*, int, bool))(0x0080f290);
inline auto ControlMapper_BindDevice = (void(__thiscall*)(class ControlMapper*, ControllerInputDriver*))(0x008065c0);
inline auto ControlMapper_SetButtonState = (void(__thiscall*)(class ControlMapper*, int, bool))(0x0080f350);
inline auto ControlMapper_ClearButtonState = (void(__thiscall*)(class ControlMapper*, int))(0x0082f390);

class ControlMapper {
public:
	struct SimButtonState {
		bool last, now;
	};
public:
	ControlMapper* prev;
	ControlMapper* next;
	ControllerInputDriver* device;
	AnalogAxis axisMap[std::to_underlying(AnalogAxis::Max)];
	float invert[std::to_underlying(AnalogAxis::Max)];
	int numButtons;
	int scheme;
	ControllerButton* buttonMap;
	SimButtonState simState[std::to_underlying(ControllerButton::Max)];
	bool enabled;
	bool initialized;
	bool allowUnlockedDeviceThisFrame;
	bool stick1ToDigital;
	float stick1ToDigitalThreshold;
	unsigned int strobeRate;
	unsigned int nextStrobe[4];
	char name[32];
	bool hasFocus;
	float sensitivity;
	float speed;
public:
	virtual ~ControlMapper();
	virtual void Initialize();
	virtual void Update();
	virtual float GetAxis(int axis) const;
	virtual bool ButtonPressed(int button, bool ignoreFocus = false);
	virtual bool ButtonReleased(int button, bool ignoreFocus = false);
	virtual bool ButtonDown(int button, bool ignoreFocus = false);
	virtual void BindDevice(ControllerInputDriver* inputDriver);
	virtual void SetButtonState(int button, bool newState);
	virtual void ClearButtonState(int button);
};

static_assert(sizeof(ControlMapper) == 0x148); 