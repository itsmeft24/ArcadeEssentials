#include <cstring>
#include "SystemInputDriver.hpp"

SystemInputDriver::SystemInputDriver() : mouseX(0), mouseY(0), hasFocus(false), keyboardEnabled(false) {
	std::memset(state, 0, sizeof(state));
}

SystemInputDriver::~SystemInputDriver() {
}

void SystemInputDriver::BeginInput() {
	for (int i = 0; i < BC_BUTTONCODES; i++) {
		state[i].last = state[i].now;
		state[i].now = false;
	}
}

void SystemInputDriver::ClearInput() {
	for (int i = 0; i < BC_BUTTONCODES; i++) {
		state[i].last = false;
		state[i].now = false;
	}
}

void SystemInputDriver::DoneInput() {
}

bool SystemInputDriver::HardwareResetPressed() {
	return false;
}

bool SystemInputDriver::HardwarePowerDown() {
	return false;
}

bool SystemInputDriver::KeyboardConnected() {
	return false;
}

bool SystemInputDriver::ButtonPressed(ButtonCode button) {
	return !state[button].last && state[button].now;
}

void SystemInputDriver::ClearButtonDown(ButtonCode button) {
	state[button].last = false;
	state[button].now = false;
}

bool SystemInputDriver::ButtonReleased(ButtonCode button) {
	return state[button].last && !state[button].now;
}

bool SystemInputDriver::ButtonDown(ButtonCode button) {
	return state[button].now;
}

bool SystemInputDriver::MouseConnected() {
	return false;
}

void SystemInputDriver::Unk() {
}

void SystemInputDriver::Unk2() {
}

void* SystemInputDriver::GetRawControllerData(int port) const {
	return nullptr;
}

void SystemInputDriver::UnlockAllControllers() {
}

void SystemInputDriver::LockPlayerToController(unsigned int player, int controllerNum) {
}

void SystemInputDriver::UnlockPlayerFromController(unsigned int playerNum) {
}

bool SystemInputDriver::ControllerLocked(unsigned int player) {
	return true;
}

bool SystemInputDriver::IsControllerLocked(unsigned int controllerIndex) {
	return true;
}

bool SystemInputDriver::UnmappedControllerLocked(unsigned int controllerNum) {
	return true;
}

int SystemInputDriver::GetLockedControllerIndex(unsigned int player) {
	return static_cast<int>(player);
}

int SystemInputDriver::GetFirstUnlockedControllerIndexStartingFrom(unsigned int startIndex) {
	return static_cast<int>(startIndex);
}

int SystemInputDriver::GetFirstUnlockedControllerIndex() {
	return -1;
}

ControllerInputDriver* SystemInputDriver::GetUnlockedController(unsigned int player) {
	return GetController(player);
}

ControllerInputDriver* SystemInputDriver::GetAssertController() {
	return GetController(0);
}

ControllerInputDriver* SystemInputDriver::GetFirstUnlockedController() {
	return nullptr;
}

unsigned int SystemInputDriver::GetNumLockedControllers() {
	return Controllers();
}

int SystemInputDriver::GetFirstUnlockedPlayer() {
	return -1;
}

int SystemInputDriver::GetPlayerIndex(ControllerInputDriver* controller) {
	return -1;
}