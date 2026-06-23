#include <algorithm>
#include <SDL3/SDL.h>
#include "SDL3ControllerDriver.hpp"
#include "../../Game/CMessage.hpp"
#include "../../Game/Genie/String.hpp"

// XInput constants retained for deadzone calculations
constexpr float XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE = 7849.0f;
constexpr float XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE = 8689.0f;
constexpr float XINPUT_GAMEPAD_TRIGGER_THRESHOLD = 30.0f;

constexpr unsigned short NINTENDO_CO_LTD = 0x057E;
constexpr unsigned short WII_REMOTE = 0x0306;

SDL3ControllerDriver::SDL3ControllerDriver(SDL_Gamepad* gamepad) : ControllerInputDriver(), m_gamepad(gamepad), m_loggedDisconnect(false) {
    Initialize();
    m_vibrationEndTime = 0;
    m_vibrationIntensity = 0;
    UpdateUIType();
}

SDL3ControllerDriver::~SDL3ControllerDriver() {
    if (m_gamepad) {
       SDL_CloseGamepad(m_gamepad); 
    }
}

void SDL3ControllerDriver::Initialize() {
    for (auto axis = 0; axis < std::to_underlying(AnalogAxis::Max); axis++) {
        float deadZonePercent = 32;
        if (axis == std::to_underlying(AnalogAxis::X1) || axis == std::to_underlying(AnalogAxis::Y1)) {
            deadZonePercent = (XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE / 32767.0f) * 100;
        }
        else if (axis == std::to_underlying(AnalogAxis::X2) || axis == std::to_underlying(AnalogAxis::Y2)) {
            deadZonePercent = (XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE / 32767.0f) * 100;
        }
        else if (axis == std::to_underlying(AnalogAxis::L2) || axis == std::to_underlying(AnalogAxis::R2)) {
            deadZonePercent = 0;
        }
        SetupAxis(static_cast<AnalogAxis>(axis), AxesTransferFunction::Linear, 1, deadZonePercent, 4);
    }
}

bool SDL3ControllerDriver::SetupAxis(AnalogAxis axis, AxesTransferFunction transferFunction, float factor, float deadZonePercent, float clampZonePercent) {
    GenerateTransferFunction(m_transferLookup[std::to_underlying(axis)], 256, transferFunction, factor, deadZonePercent, clampZonePercent);
    return true;
}

bool SDL3ControllerDriver::Connected() {
    return m_gamepad != nullptr && SDL_GamepadConnected(m_gamepad);
}

void SDL3ControllerDriver::Reconnect(SDL_Gamepad* newGamepad) {
    if (m_gamepad) {
        SDL_CloseGamepad(m_gamepad);
    }
    m_gamepad = newGamepad;
    m_loggedDisconnect = false;

    // Added this to fix hotplugging controllers not updating the UI type.
    // If P1 was PlayStation and P2 was Xbox, If P1 disconnected then P2, then Xbox reconnecting wouldn't set P1 to Xbox UI, now it does.
    UpdateUIType();
}

void SDL3ControllerDriver::BeginInput() {
    ControllerInputDriver::BeginInput();

    if (m_gamepad && SDL_GamepadConnected(m_gamepad)) {
       SetState(ControllerButton::Left, SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_DPAD_LEFT));
       SetState(ControllerButton::Down, SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_DPAD_DOWN));
       SetState(ControllerButton::Right, SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT));
       SetState(ControllerButton::Up, SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_DPAD_UP));

       SetState(ControllerButton::Start, SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_START));
       SetState(ControllerButton::Select, SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_BACK));

       SetState(ControllerButton::Stick1, SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_LEFT_STICK));
       SetState(ControllerButton::Stick2, SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_RIGHT_STICK));

       if (m_uiType == UIControllerType::GameCubeController) {
           // SDL3 Goes out of its way to screw up the binds on GameCube controllers just because. So we have to do this.
           SetState(ControllerButton::Square, SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_NORTH));
           SetState(ControllerButton::Cross, SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_WEST));
           SetState(ControllerButton::Circle, SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_SOUTH));
           SetState(ControllerButton::Triangle, SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_EAST));
           SetState(ControllerButton::Back, SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_SOUTH));
       }
       else {
           SetState(ControllerButton::Square, SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_WEST));
           SetState(ControllerButton::Cross, SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_SOUTH));
           SetState(ControllerButton::Circle, SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_EAST));
           SetState(ControllerButton::Triangle, SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_NORTH));
           SetState(ControllerButton::Back, SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_EAST));
       }
       
       SetState(ControllerButton::L1, SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER));
       SetState(ControllerButton::R1, SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER));

       constexpr auto TRIGGER_THRESHOLD = static_cast<short>(30.0 / 255.0 * 32767.0);
       Sint16 axisL2 = SDL_GetGamepadAxis(m_gamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
       Sint16 axisR2 = SDL_GetGamepadAxis(m_gamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);

       SetState(ControllerButton::L2, axisL2 > TRIGGER_THRESHOLD);
       SetState(ControllerButton::R2, axisR2 > TRIGGER_THRESHOLD);

       Sint16 axisLX = SDL_GetGamepadAxis(m_gamepad, SDL_GAMEPAD_AXIS_LEFTX);
       Sint16 axisLY = SDL_GetGamepadAxis(m_gamepad, SDL_GAMEPAD_AXIS_LEFTY);
       Sint16 axisRX = SDL_GetGamepadAxis(m_gamepad, SDL_GAMEPAD_AXIS_RIGHTX);
       Sint16 axisRY = SDL_GetGamepadAxis(m_gamepad, SDL_GAMEPAD_AXIS_RIGHTY);

       SetStick(m_transferLookup[std::to_underlying(AnalogAxis::X1)], 256, AnalogAxis::X1, std::clamp((axisLX / 256) + 128, 0, 255));
       SetStick(m_transferLookup[std::to_underlying(AnalogAxis::Y1)], 256, AnalogAxis::Y1, std::clamp((-axisLY / 256) + 128, 0, 255));

       SetStick(m_transferLookup[std::to_underlying(AnalogAxis::X2)], 256, AnalogAxis::X2, std::clamp((axisRX / 256) + 128, 0, 255));
       SetStick(m_transferLookup[std::to_underlying(AnalogAxis::Y2)], 256, AnalogAxis::Y2, std::clamp((-axisRY / 256) + 128, 0, 255));

       SetStick(m_transferLookup[std::to_underlying(AnalogAxis::L2)], 256, AnalogAxis::L2, ((axisL2 >> 7) / 2) + 128);
       SetStick(m_transferLookup[std::to_underlying(AnalogAxis::R2)], 256, AnalogAxis::R2, ((axisR2 >> 7) / 2) + 128);

       SimulatePointer();
       UpdateAxisPairs();
    }
}

void SDL3ControllerDriver::DoneInput() {
    if (m_vibrationEndTime > 0 && m_vibrationEndTime <= *reinterpret_cast<unsigned int*>(0x01925e38)) {
        ClearVibration(true);
    }
}

void SDL3ControllerDriver::Vibration(bool allow) {
    ControllerInputDriver::Vibration(allow);

    if ((!m_allowVibration || !*reinterpret_cast<bool*>(0x0185ddf5)) && m_vibrationIntensity != 0) {
       ClearVibration(true);
    }
}

bool SDL3ControllerDriver::SetVibration(unsigned int intensity, VibratePan pan) {
    if ((!m_allowVibration || !*reinterpret_cast<bool*>(0x0185ddf5)) && intensity != 0) {
       return false;
    }

    if (!m_gamepad || !SDL_GamepadConnected(m_gamepad)) return false;

    Uint16 lowFreq = static_cast<Uint16>(65535 * ((intensity & 0xFF) / 200.0f));
    Uint16 highFreq = ((intensity & 256) != 0) ? 65535 : 0;

    if (SDL_RumbleGamepad(m_gamepad, lowFreq, highFreq, 3600000)) {
       m_vibrationIntensity = intensity;
       m_vibrationEndTime = 0;
       return true;
    }

    return false;
}

bool SDL3ControllerDriver::SetVibrationDuration(unsigned int durationMilliseconds, unsigned int intensity, VibratePan pan) {
    if (SetVibration(intensity, pan)) {
       m_vibrationEndTime = *reinterpret_cast<unsigned int*>(0x01925e38) + durationMilliseconds;
       return true;
    }

    return false;
}

bool SDL3ControllerDriver::ClearVibration(bool force) {
    if (m_vibrationIntensity == 0) {
        return false;
    }
    if (m_vibrationEndTime == 0 || force) {
        if (m_gamepad && SDL_GamepadConnected(m_gamepad)) {
            SDL_RumbleGamepad(m_gamepad, 0, 0, 0);
            m_vibrationIntensity = 0;
            m_vibrationEndTime = 0;
            return true;
        }
        else {
            return false;
        }
    }
    return true;
}

bool SDL3ControllerDriver::GetVibration(unsigned int& intensity) {
    if (m_vibrationIntensity == 0) {
       return false;
    }
    intensity = m_vibrationIntensity;
    return true;
}

bool SDL3ControllerDriver::AnyButtonPressed() {
    return false;
}

void SDL3ControllerDriver::SimulatePointer() {
    float x = m_stick[std::to_underlying(AnalogAxis::X2)] * 0.025f;
    float y = m_stick[std::to_underlying(AnalogAxis::Y2)] * 0.025f;

    Vector2 newPointer{};
    newPointer.X(std::clamp(m_pointer.X() + x, -1.0f, 1.0f));
    newPointer.Y(std::clamp(m_pointer.Y() - y, -1.0f, 1.0f));

    m_pointer = newPointer;
    m_horizon = Vector2();
    m_pointerValid = true;
}

// ITSMEFT24 READ THIS.
// When you implement on the fly image swapping, this is the function that should trigger it.
// If this is called, the images need to be updated in game.
// Eg. Calling this triggers image swap.
void SDL3ControllerDriver::UpdateUIType() {
    if (m_gamepad == nullptr) {
        return;
    }

    switch (SDL_GetRealGamepadType(m_gamepad)) {
    case SDL_GAMEPAD_TYPE_XBOX360:
    case SDL_GAMEPAD_TYPE_XBOXONE:
        m_uiType = UIControllerType::Xbox360Controller;
        break;
    case SDL_GAMEPAD_TYPE_PS3:
    case SDL_GAMEPAD_TYPE_PS4:
    case SDL_GAMEPAD_TYPE_PS5:
        m_uiType = UIControllerType::DualShock3;
        break;
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_PRO:
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_PAIR:
        m_uiType = UIControllerType::WiiClassicController;
        break;
    case SDL_GAMEPAD_TYPE_GAMECUBE:
        m_uiType = UIControllerType::GameCubeController;
        break;
    default:
        break;
    }

    std::uint16_t vendor = SDL_GetGamepadVendor(m_gamepad);
    std::uint16_t product = SDL_GetGamepadProduct(m_gamepad);

    if ((vendor == NINTENDO_CO_LTD && product == WII_REMOTE)) {
        m_uiType = UIControllerType::WiiRemote;
    }
}

const char* SDL3ControllerDriver::Identify() {
    return "SDL3 Controller";
}
