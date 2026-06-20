#include "ControllerInputDriver.hpp"

ControllerInputDriver::ControllerInputDriver() {
	m_allowVibration = true;
	m_headSetStatus = 0;
	m_headSetPort = -1;
	m_bInvalidController = false;
	m_iApplicationControllerID = -1;
	m_controller = nullptr;
	m_pointerValid = false;
	m_allowConnection = true;
	m_pSharedController = nullptr;

	for (auto i = 0; i < std::to_underlying(ControllerButton::Max); i++) {
		m_state[i].last = false;
		m_state[i].now = false;
	}
	for (auto i = 0; i < std::to_underlying(AnalogAxis::Max); i++) {
		m_stick[i] = 0.0f;
		m_rawStick[i] = 0.0f;
		m_axisPairs[i].axis1 = AnalogAxis::Invalid;
	}
	for (auto i = 0; i < sizeof(m_accelerometer) / sizeof(Vector3); i++) {
		m_accelerometer[i].x = 0;
		m_accelerometer[i].y = -1;
		m_accelerometer[i].z = 0;
	}
	m_pointer.x = 0;
	m_pointer.y = 0;
	m_horizon.x = 0;
	m_horizon.y = 0;

	m_uiType = UIControllerType::Xbox360Controller;
}

ControllerInputDriver::~ControllerInputDriver() {
}

bool ControllerInputDriver::ButtonPressed(ControllerButton button, bool bCheckShared) {
	bool ret = (m_state[std::to_underlying(button)].last == false && m_state[std::to_underlying(button)].now == true);
	if (bCheckShared && m_pSharedController != nullptr) {
		ret |= m_pSharedController->ButtonPressed(button, false);
	}
	return ret;
}

bool ControllerInputDriver::ButtonReleased(ControllerButton button, bool bCheckShared) {
	bool ret = (m_state[std::to_underlying(button)].last == true && m_state[std::to_underlying(button)].now == false);
	if (bCheckShared && m_pSharedController != nullptr) {
		ret |= m_pSharedController->ButtonPressed(button, false);
	}
	return ret;
}

bool ControllerInputDriver::ButtonDown(ControllerButton button, bool bCheckShared) {
	bool ret = m_state[std::to_underlying(button)].now;
	if (bCheckShared && m_pSharedController != nullptr) {
		ret |= m_pSharedController->ButtonDown(button, false);
	}
	return ret;
}

void ControllerInputDriver::BeginInput() {
	for (auto i = 0; i < std::to_underlying(ControllerButton::Max); i++) {
		m_state[i].last = m_state[i].now;
	}
}

void ControllerInputDriver::SetSecondaryController(ControllerInputDriver* controller) {
	if (controller != nullptr){
		if (controller->Connected() && controller != this) {
			m_controller = controller;
			return;
		}
	}
	m_controller = nullptr;
}