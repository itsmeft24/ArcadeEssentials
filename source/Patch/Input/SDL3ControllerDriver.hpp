#pragma once
#include <SDL3/SDL_gamepad.h>
#include "ControllerInputDriver.hpp"

class SDL3ControllerDriver : public ControllerInputDriver {
public:
	SDL_Gamepad* m_gamepad;
	float m_transferLookup[19][256];
	unsigned int m_vibrationEndTime;
	unsigned int m_vibrationIntensity;
	bool m_loggedDisconnect;
public:
	SDL3ControllerDriver(SDL_Gamepad* gamepad);
	virtual ~SDL3ControllerDriver() override;
	virtual void Initialize() override;
	inline virtual const char* DeviceName() const override {
		return "SDL3 Controller";
	}
	virtual bool SetupAxis(AnalogAxis axis, AxesTransferFunction transferFunction, float factor, float deadZonePercent, float clampZonePercent)  override;
	void Reconnect(SDL_Gamepad* newGamepad);
	virtual bool Connected() override;
	inline virtual int GetUserID() override { return 0; }
	virtual void BeginInput() override;
	virtual void DoneInput() override;
	virtual void Vibration(bool allow) override;
	virtual bool SetVibration(unsigned int intensity, VibratePan pan) override;
	virtual bool SetVibrationDuration(unsigned int durationMilliseconds, unsigned int intensity, VibratePan pan) override;
	virtual bool ClearVibration(bool force = false) override;
	virtual bool GetVibration(unsigned int& intensity) override;
	virtual bool AnyButtonPressed() override;

	void SimulatePointer();

	virtual const char* Identify() override;
private:
	void UpdateUIType();
};