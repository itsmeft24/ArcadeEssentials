#pragma once
#include <cstdint>
#include <utility>
#include "../../Game/Utils/Vector3.hpp"
#include "../../Game/Utils/Vector2.hpp"

inline auto ControllerInputDriver_GenerateTransferFunction = (bool(__thiscall*)(class ControllerInputDriver*, float*, unsigned int, enum class AxesTransferFunction, float, float, float))(0x00810350);

enum class ControllerButton {
	Invalid,
	Left,
	Down,
	Right,
	Up,
	Square,
	Cross,
	Circle,
	Triangle,
	L1,
	R1,
	L2,
	R2,
	Start,
	Select,
	Stick1,
	Stick2,
	Back,
	ArcadeCoin,
	ArcadeUnk,
	ArcadeBill,
	ArcadeTestMode,
	ArcadeService,
	ArcadeVolumeUp,
	ArcadeVolumeDown,
	ArcadeMenuConfirm,
	ArcadeUnk2,
	ArcadeUnk3,
	ArcadeUnk4,
	ArcadeChatter,
	ArcadeUnk5,
	Unk6,
	Unk7,
	Unk8,
	Unk9,
	Unk10,
	Unk11,
	Escape,
	Max
};

enum class AnalogAxis {
	Invalid,
	X1,
	Y1,
	X2,
	Y2,
	Throttle,
	Rudder,
	Left,
	Right,
	Up,
	Down,
	Triangle,
	Circle,
	Cross,
	Square,
	L1,
	R1,
	L2,
	R2,
	Max,
};

enum class AxesTransferFunction {
	Linear,
	Parabolic,
	Exponential,
	Power,
};

enum class VibratePan {
	Center,
	Left,
	Right
};

enum class UIControllerType : unsigned char {
	Xbox360Controller = 0,
	DualShock3,
	WiiRemote,
	WiiRemoteNunchuck,
	WiiClassicController,
	GameCubeController,
};

inline std::string_view SuffixForCT(UIControllerType self) {
	switch (self) {
	case UIControllerType::Xbox360Controller:
		return "";
	case UIControllerType::DualShock3:
		return "_ps3";
	case UIControllerType::WiiRemote:
		return "_wii";
	case UIControllerType::WiiRemoteNunchuck: // WPAD_DEV_FREESTYLE, On Wii they just don't have a suffix for this.
		return "_wii";
	case UIControllerType::WiiClassicController:
		return "_wcc";
	case UIControllerType::GameCubeController:
		return "_gcn";
	}
}

inline std::string_view LoadTipSuffixForCT(UIControllerType self) {
	switch (self) {
	case UIControllerType::Xbox360Controller:
		return "";
	case UIControllerType::DualShock3:
		return "_ps3";
	case UIControllerType::WiiRemote: // WPAD_DEV_CORE, On Wii they just don't have a suffix for this.
		return "_WiiRemote";
	case UIControllerType::WiiRemoteNunchuck: 
		return "_Nunchuk";
	case UIControllerType::WiiClassicController:
		return "_WiiClassic";
	case UIControllerType::GameCubeController:
		return "_Gamecube";
	}
}

inline std::string_view ActionMapPlatformForCT(UIControllerType self) {
	switch (self) {
	case UIControllerType::Xbox360Controller:
	case UIControllerType::DualShock3:
		return "X360";
	case UIControllerType::WiiRemote: // WPAD_DEV_CORE
		return "Wii-wheel";
	case UIControllerType::WiiRemoteNunchuck:
		return "Wii-nunchuk";
	case UIControllerType::WiiClassicController:
		return "Wii-classic";
	case UIControllerType::GameCubeController:
		return "Wii-gamecube";
	}
}

inline std::string_view XMLNameForCT(UIControllerType self) {
	switch (self) {
	case UIControllerType::Xbox360Controller:
	case UIControllerType::DualShock3:
		return "Controls_ButtonAction.xml";
	case UIControllerType::WiiRemote: // WPAD_DEV_CORE
		return "Controls_wiibasic.xml";
	case UIControllerType::WiiRemoteNunchuck:
		return "Controls_wiinunchuk.xml";
	case UIControllerType::WiiClassicController:
		return "Controls_ButtonAction.xml"; // "Controls_wiiclassic.xml";
	case UIControllerType::GameCubeController:
		return "Controls_wiiGameCube.xml";
	}
}

struct ButtonState {
	bool last;
	bool now;
};

struct AxisPair {
	AnalogAxis axis1;
	AnalogAxis axis2;
	float deadZonePercent;
	float clampZonePercent;
};

class ControllerInputDriver {
public:
	unsigned int m_bInvalidController;
	int m_iApplicationControllerID;
	ButtonState	m_state[std::to_underlying(ControllerButton::Max)];
	float m_stick[std::to_underlying(AnalogAxis::Max)];
	float m_rawStick[std::to_underlying(AnalogAxis::Max)];
	AxisPair m_axisPairs[std::to_underlying(AnalogAxis::Max)];
	Vector3 m_accelerometer[3];
	Vector2 m_pointer;
	Vector2 m_horizon;
	bool m_pointerValid;
	bool m_allowVibration;
	bool m_allowConnection;
	bool m_inFirstPerson;
	bool m_centerView;
	UIControllerType m_uiType;
	int m_headSetStatus;
	int m_headSetPort;
	ControllerInputDriver* m_pSharedController;
	ControllerInputDriver* m_controller;
public:
	ControllerInputDriver();
	ControllerInputDriver(const ControllerInputDriver&) = delete;
	ControllerInputDriver& operator=(const ControllerInputDriver&) = delete;

	virtual ~ControllerInputDriver();
	virtual void Initialize() = 0;
	virtual const char* DeviceName() const = 0;
	virtual bool SetupAxis(AnalogAxis axis, AxesTransferFunction transferFunction, float factor, float deadZonePercent, float clampZonePercent) = 0;
	virtual bool Connected() = 0;
	virtual int GetUserID() = 0;
	virtual bool ButtonPressed(ControllerButton button, bool bCheckShared = true);
	virtual bool ButtonReleased(ControllerButton button, bool bCheckShared = true);
	virtual bool ButtonDown(ControllerButton button, bool bCheckShared = true);
	inline virtual struct AccChannels* GetAccelerometerChannels() { return nullptr; }
	inline virtual bool IsPointerEnabled() { return false; }
	inline virtual void EnablePointer(bool enabled) {}
	inline virtual bool ButtonPeek(ControllerButton button) { return false; }
	inline virtual void Vibration(bool allow) { m_allowVibration = allow; }
	inline virtual std::uint8_t GetKey(void) { return 0; }
	inline virtual bool IsShiftPressed(void) { return false; }
	inline virtual bool ShiftReleased(void) { return false; }
	inline virtual bool IsCapsLocked(void) { return false; }
	inline virtual bool CapsUnLocked(void) { return false; }
	inline virtual bool IsUpArrow(void) { return false; }
	inline virtual bool IsDownArrow(void) { return false; }
	inline virtual bool IsLeftArrow(void) { return false; }
	inline virtual bool IsRightArrow(void) { return false; }
	inline virtual bool SetVibration(unsigned int intensity, VibratePan pan = VibratePan::Center) { return false; }
	inline virtual bool SetVibrationDuration(unsigned int durationMilliseconds, uint_fast32_t intensity, VibratePan pan = VibratePan::Center) { return false; }
	inline virtual bool ClearVibration(bool force = false) { return false; }
	inline virtual bool GetVibration(unsigned int& intensity) { return false; }
	virtual void BeginInput();
	inline virtual void DoneInput() {}
	inline virtual void Activate(bool active) {}
	inline virtual bool ObserveFocus() { return false; }
	virtual void SetSecondaryController(ControllerInputDriver* controller);
	virtual bool AnyButtonPressed() = 0;
	virtual const char* Identify() = 0;
public:
	inline void SetSharedController(ControllerInputDriver* sharedController) {
		m_pSharedController = sharedController;
	}
	inline ControllerInputDriver* GetSharedController() {
		return m_pSharedController;
	}
	inline bool GenerateTransferFunction(float* transferLookup, unsigned int entries, AxesTransferFunction transferFunction, float factor, float deadZonePercent, float clampZonePercent) {
		return ControllerInputDriver_GenerateTransferFunction(this, transferLookup, entries, transferFunction, factor, deadZonePercent, clampZonePercent);
	}
	inline void SetState(ControllerButton button, bool state) {
		m_state[std::to_underlying(button)].now = state;
	}
	inline void SetStick(float* transferLookup, unsigned int entries, AnalogAxis axis, int index) {
		if (m_axisPairs[std::to_underlying(axis)].axis1 == AnalogAxis::Invalid && index < static_cast<int>(entries)) {
			m_stick[std::to_underlying(axis)] = transferLookup[index];
		}
		m_rawStick[std::to_underlying(axis)] = static_cast<float>(index - entries / 2.0f) / (entries / 2.0f);
	}
	inline void UpdateAxisPairs() {
		for (auto i = 0; i < std::to_underlying(AnalogAxis::Max); i++) {
			if (i != std::to_underlying(AnalogAxis::Invalid) && std::to_underlying(m_axisPairs[i].axis1) == i) {
				float axis1 = m_rawStick[i];
				float origAxis1 = axis1;
				float axis2 = m_rawStick[std::to_underlying(m_axisPairs[i].axis2)];
				float origAxis2 = axis2;

				Vector3 stickVector = Vector3(axis1, axis2, 0.0f);
				float length = stickVector.Length();
				stickVector.Normalize();

				if (length < m_axisPairs[i].deadZonePercent) {
					length = 0.0f;
				}
				else if (length > m_axisPairs[i].clampZonePercent) {
					length = 1.0f;
				}
				else {
					length = (length - m_axisPairs[i].deadZonePercent) / (m_axisPairs[i].clampZonePercent - m_axisPairs[i].deadZonePercent);
				}

				stickVector = length * stickVector.Normalized();
				m_stick[i] = stickVector.X();
				m_stick[std::to_underlying(m_axisPairs[i].axis2)] = stickVector.Y();
			}
		}
	}
};

static_assert(sizeof(ControllerInputDriver) == 0x26C);