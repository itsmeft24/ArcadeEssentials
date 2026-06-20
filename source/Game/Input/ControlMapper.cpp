#include "ControlMapper.hpp"

ControlMapper::~ControlMapper() {
	ControlMapper_Destructor(this);
}

void ControlMapper::Initialize() {
	ControlMapper_Initialize(this);
}

void ControlMapper::Update() {
	ControlMapper_Update(this);
}

float ControlMapper::GetAxis(int axis) const {
	return ControlMapper_GetAxis(this, axis);
}

bool ControlMapper::ButtonPressed(int button, bool ignoreFocus) {
	return ControlMapper_ButtonPressed(this, button, ignoreFocus);
}

bool ControlMapper::ButtonReleased(int button, bool ignoreFocus) {
	return ControlMapper_ButtonReleased(this, button, ignoreFocus);
}

bool ControlMapper::ButtonDown(int button, bool ignoreFocus) {
	return ControlMapper_ButtonDown(this, button, ignoreFocus);
}

void ControlMapper::BindDevice(ControllerInputDriver * inputDriver) {
	ControlMapper_BindDevice(this, inputDriver);
}

void ControlMapper::SetButtonState(int button, bool newState) {
	ControlMapper_SetButtonState(this, button, newState);
}

void ControlMapper::ClearButtonState(int button) {
	ControlMapper_ClearButtonState(this, button);
}