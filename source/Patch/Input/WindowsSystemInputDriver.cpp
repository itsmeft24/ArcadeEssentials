#include <cstdint>
#include <array>
#include <thread>
#include <atomic>
#include <cstring>
#include <Windows.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_events.h>
#include "WindowsSystemInputDriver.hpp"
#include "SDL3ControllerDriver.hpp"
#include "KeyControllerInputDriver.hpp"
#include "../../Game/CMessage.hpp"
#include "../../Game/FrontEnd/CarsFrontEnd.hpp"
#include "../../pentane.hpp"

inline auto CMasterTimer_GetOSTime = (std::uint32_t(_cdecl*)())(0x00770280);
inline auto _DirectInput8Create = (HRESULT(__stdcall*)(HINSTANCE, DWORD, const IID&, LPVOID*, LPUNKNOWN))(0x013c0fc4);

inline const DIDATAFORMAT* _c_dfDIMouse2 = reinterpret_cast<const DIDATAFORMAT*>(0x016adb6c);
inline const DIDATAFORMAT* _c_dfDIKeyboard = reinterpret_cast<const DIDATAFORMAT*>(0x016ad964);

std::atomic<std::uint32_t> LAST_CONTROLLER_CHECK_TIMESTAMP = 0;

// Helper function to see if a newly discovered SDL Gamepad is already active in one of our slots
bool IsSDLGamepadAlreadyMapped(WindowsSystemInputDriver* driver, SDL_JoystickID id) {
	for (auto i = 0; i < MAX_CONTROLLERS; i++) {
		if (driver->controller[i] != nullptr) {
			if (std::string_view(driver->controller[i]->Identify()) == "SDL3 Controller") {
				SDL3ControllerDriver* device = reinterpret_cast<SDL3ControllerDriver*>(driver->controller[i]);
				if (device->m_gamepad && SDL_GetGamepadID(device->m_gamepad) == id) {
					return true;
				}
			}
		}
	}
	return false;
}

std::optional<int> WindowsSystemInputDriver::FindKBM() {
	for (auto i = 0; i < MAX_CONTROLLERS; i++) {
		if (controller[i] != nullptr && controller[i] == d_pKeyboard) {
			return i;
		}
	}
	return std::nullopt;
}

std::optional<int> WindowsSystemInputDriver::FindFirstAvailableControllerSlot() {
	for (auto i = 0; i < MAX_CONTROLLERS; i++) {
		if (this->controller[i] == nullptr) {
			return i;
		}
	}
	return std::nullopt;
}

void WindowsSystemInputDriver::SwapControllers(int i, int j) {
	ControllerInputDriver* oldI = controller[i];
	controller[i] = controller[j];
	controller[j] = oldI;
}

void WindowsSystemInputDriver::BeginInputHook() {
	// If KBM is open and player 0 disconnected, we can move the keyboard there.
	// (Otherwise, we'll be waiting for a new controller to connect.)
	if (controller[0] == nullptr) {
		auto slot = FindKBM();
		if (slot.has_value()) {
			if (!IsControllerLocked(slot.value())) {
				SwapControllers(0, slot.value());
				LockPlayerToController(0, 0);
			}
		}
		else {
			controller[0] = d_pKeyboard;
			LockPlayerToController(0, 0);
		}
	}

	for (int i = 0; i < BC_BUTTONCODES; i++) {
		state[i].last = state[i].now;
	}

	if (hasFocus) {
		if (m_pKeyboard->GetDeviceState(sizeof(m_cKeyboardBuffer), m_cKeyboardBuffer) < 0) {
			m_pKeyboard->Acquire();
			return;
		}

		unsigned char* dInputKeyCode = reinterpret_cast<unsigned char*>(0x0185efa8);
		for (int i = 0; i < BC_BUTTONCODES - 1; i++) {
			if (dInputKeyCode[i] != 0) {
				state[i].now = ((m_cKeyboardBuffer[dInputKeyCode[i]] & 0x80) != 0);
			}
		}
	}

	if (state[BC_LALT].now || state[BC_RALT].now) {
		state[BC_ENTER].now = false;
		state[BC_NUM_ENTER].now = false;
	}

	std::uint32_t current = CMasterTimer_GetOSTime();

	SDL_UpdateGamepads();

	// Periodic check for Controller Drops & Connections (~Every 2 Seconds)
	if ((current - LAST_CONTROLLER_CHECK_TIMESTAMP) >= 2000) {

		// Identify disconnected controllers & log them (keep slot alive for reconnect)
		for (int i = 0; i < MAX_CONTROLLERS; i++) {
			if (controller[i] != nullptr) {
				if (std::string_view(controller[i]->Identify()) == "SDL3 Controller") {
					SDL3ControllerDriver* sdl = reinterpret_cast<SDL3ControllerDriver*>(controller[i]);
					if (!sdl->Connected() && !sdl->m_loggedDisconnect) {
						sdl->m_loggedDisconnect = true;
						logger::log_format("[WindowsSystemInputDriver::BeginInput] SDL Controller Disconnected! (Index: {})", i);

						if (IsControllerLocked(i)) {
							ShowControllerPullPacket packet = { .popupMessage = "Popup_Title_Attention^Reconnect_Controller_fmt", .playerIndex = GetPlayerIndex(sdl), .type = 1};
							(*g_MessageDispatcher)->SendMessageToAll("ShowControllerPullMessage", &packet, 0);
						}
					}
				}
			}
		}

		// Identify & attach newly connected controllers
		int gamepadCount = 0;
		SDL_JoystickID* joysticks = SDL_GetGamepads(&gamepadCount);
		if (joysticks) {
			for (int i = 0; i < gamepadCount; i++) {
				if (!IsSDLGamepadAlreadyMapped(this, joysticks[i])) {
					SDL_Gamepad* newGamepad = SDL_OpenGamepad(joysticks[i]);
					if (!newGamepad) continue;

					// Fill previously disconnected slots rather than making new ones.
					int targetSlot = -1;
					for (int s = 0; s < MAX_CONTROLLERS; s++) {
						if (controller[s] != nullptr && std::string_view(controller[s]->Identify()) == "SDL3 Controller") {
							SDL3ControllerDriver* sdl = reinterpret_cast<SDL3ControllerDriver*>(controller[s]);
							if (!sdl->Connected()) {
								targetSlot = s;
								break;
							}
						}
					}

					if (targetSlot != -1) {
						SDL3ControllerDriver* sdl = reinterpret_cast<SDL3ControllerDriver*>(controller[targetSlot]);
						sdl->Reconnect(newGamepad);
						SDL_UpdateGamepads();

						logger::log_format("[WindowsSystemInputDriver::BeginInput] SDL Controller Reconnected! Restored to index: {}", targetSlot);
						if (FrontEnd_InFrontend(*g_FrontEnd) && IsControllerLocked(targetSlot)) {
							logger::log_format("[WindowsSystemInputDriver::BeginInput] Player Index: {}", GetPlayerIndex(sdl));
							ShowControllerPullPacket showPacket = { .popupMessage = "Popup_Title_Attention^Reconnect_Controller_fmt", .playerIndex = GetPlayerIndex(sdl), .type = 2 };
							(*g_MessageDispatcher)->SendMessageToAll("ShowControllerPullMessage", &showPacket, 0);
						}
						else if (IsControllerLocked(targetSlot)) {
							logger::log_format("[WindowsSystemInputDriver::BeginInput] Player Index: {}", GetPlayerIndex(sdl));
							HideControllerPullPacket hidePacket = { .playerIndex = GetPlayerIndex(sdl) };
							(*g_MessageDispatcher)->SendMessageToAll("HideControllerPullMessage", &hidePacket, 0);
						}
					}
					else {
						if (auto slot = FindFirstAvailableControllerSlot(); slot.has_value()) {
							controller[slot.value()] = new SDL3ControllerDriver(newGamepad);
							controllers++;
							logger::log_format("[WindowsSystemInputDriver::BeginInput] SDL Controller Connected! Mapped to index: {}", slot.value());
						}
						else {
							logger::log_format("[WindowsSystemInputDriver::BeginInput] No open controller slots for new SDL Controller! Skipping...");
							SDL_CloseGamepad(newGamepad);
							break;
						}
					}
				}
			}
			SDL_free(joysticks);
		}

		LAST_CONTROLLER_CHECK_TIMESTAMP = CMasterTimer_GetOSTime();
	}

	for (int i = 0; i < 11; i++) {
		if (controller[i] != nullptr) {
			controller[i]->BeginInput();
		}
	}

	if (m_pMouse != nullptr) {
		DIMOUSESTATE2 dims2{};
		if (m_pMouse->GetDeviceState(sizeof(DIMOUSESTATE2), &dims2) < 0) {
			m_pMouse->Acquire();
		}
		else {
			mouseX = dims2.lX;
			mouseY = dims2.lY;
			if (abs(mouseX) * 3 < abs(mouseY)) {
				mouseX = 0;
			}
			else if (abs(mouseY) * 3 < abs(mouseX)) {
				mouseY = 0;
			}

			if (GetSystemMetrics(SM_SWAPBUTTON) != 0) {
				state[BC_MOUSELEFT].now = (dims2.rgbButtons[1] & 0x80) != 0;
				state[BC_MOUSERIGHT].now = (dims2.rgbButtons[0] & 0x80) != 0;
			}
			else {
				state[BC_MOUSELEFT].now = (dims2.rgbButtons[0] & 0x80) != 0;
				state[BC_MOUSERIGHT].now = (dims2.rgbButtons[1] & 0x80) != 0;
			}
			state[BC_MOUSEMIDDLE].now = (dims2.rgbButtons[2] & 0x80) != 0;
		}
	}
}

bool WindowsSystemInputDriver::Initialize(HINSTANCE hInst, HWND hWnd) {
	this->hWnd = hWnd;

	SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_GAMECUBE, "1");
	SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_WII, "1");
	SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_SWITCH2, "1");
	if (SDL_Init(SDL_INIT_GAMEPAD) < 0) {
		logger::log_format("[WindowsSystemInputDriver::Initialize] SDL_Init Failed: {}", SDL_GetError());
		return false;
	}

	// Populate initial gamepads via SDL
	int gamepadCount = 0;
	SDL_JoystickID* joysticks = SDL_GetGamepads(&gamepadCount);
	if (joysticks) {
		for (int i = 0; i < gamepadCount && this->controllers < MAX_CONTROLLERS; ++i) {
			SDL_Gamepad* gamepad = SDL_OpenGamepad(joysticks[i]);
			if (gamepad) {
				this->controller[this->controllers] = new SDL3ControllerDriver(gamepad);
				logger::log_format("[WindowsSystemInputDriver::Initialize] SDL Controller: {} mapped to index: {}!", SDL_GetGamepadName(gamepad), this->controllers);
				this->controllers++;
			}
		}
		SDL_free(joysticks);
	}

	if (_DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8A, reinterpret_cast<void**>(&this->dInput), NULL) != DI_OK) {
		logger::log_format("[WindowsSystemInputDriver::Initialize] _DirectInput8Create Failed!");
	}

	if (this->controllers < MAX_CONTROLLERS && this->controller[this->controllers] == nullptr) {
		char sMyDocumentsPath[260]{};
		strcpy_s(sMyDocumentsPath, sizeof(sMyDocumentsPath), reinterpret_cast<const char*>(0x018d3138));
		if (sMyDocumentsPath[0] == 0) {
			strcpy_s(sMyDocumentsPath, sizeof(sMyDocumentsPath), ".\\data_dx\\buttmap\\");
		}
		strcat_s(sMyDocumentsPath, "keymap.ini");
		this->controller[this->controllers] = new KeyControllerInputDriver(this, sMyDocumentsPath);
		this->d_pKeyboard = this->controller[this->controllers];
		this->controllers++;
	}

	if (FAILED(this->dInput->CreateDevice(GUID_SysMouse, &this->m_pMouse, NULL))) {
		return false;
	}
	if (FAILED(this->m_pMouse->SetDataFormat(_c_dfDIMouse2))) {
		return false;
	}
	this->m_pMouse->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	this->m_pMouse->Acquire();
	if (FAILED(this->dInput->CreateDevice(GUID_SysKeyboard, &this->m_pKeyboard, NULL))) {
		return false;
	}
	if (FAILED(this->m_pKeyboard->SetDataFormat(_c_dfDIKeyboard))) {
		return false;
	}
	this->m_pKeyboard->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND | DISCL_NOWINKEY);
	this->m_pKeyboard->Acquire();
	return true;
}

void __fastcall WindowsSystemInputDriver_BeginInput(WindowsSystemInputDriver* _this) {
	_this->BeginInputHook();
}

bool __fastcall WindowsSystemInputDriver_Initialize(WindowsSystemInputDriver* _this, std::uintptr_t edx, HINSTANCE hInst, HWND hWnd) {
	return _this->Initialize(hInst, hWnd);
}