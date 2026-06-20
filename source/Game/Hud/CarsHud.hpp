#pragma once
#include "../CMessage.hpp"
#include "../Scaleform/Wrapper/Movie.hpp"
#include "../Scaleform/GFxValue.hpp"
#include "../CActor.hpp"

inline auto CarsHud_TrickCallout = (void(__thiscall*)(class CarsHud*, CActor*, const char*, const char*, std::uint32_t))(0x00552c40);
inline auto HudActiveInfo_SetTextAndPlayAnim = (void(__thiscall*)(void*, char*, unsigned int, char*, const char*, bool))(0x0054eb80);

constexpr auto WEAPON_TUTORIAL_LIVE_TIME = 0x40200000;

class HudGauge;
class HudClocks;
class HudPlace;
class HudScore;
class HudActiveInfo {
public:
	void SetTextAndPlayAnim(char* param_1, unsigned int param_2, char* param_3, const char* param_4, bool param_5) {
		HudActiveInfo_SetTextAndPlayAnim(this, param_1, param_2, param_3, param_4, param_5);
	}
};
class HudSkateReticule;
class HudTeamMarkers;
class Hud1Up;
class HudTrickCallout;
class HudHealthBar;
class HudMissionMeter;
class HudTutorialButtonPrompt;
class HudPosition;
class HudFrame;
class HudTimer;

class HudCountdown;

class HudSplitScreen {
public:
	Flash::Movie* m_movie;
	HudGauge* m_hudGuage;
	HudClocks* m_hudClocks;
	HudPlace* m_hudPlace;
	HudScore* m_hudScore;
	HudActiveInfo* m_hudActiveInfo;
	HudSkateReticule* m_hudSkateReticule;
	HudTeamMarkers* m_hudTeamMarkers;
	Hud1Up* m_hud1Up;
	HudTrickCallout* m_hudTrickCallout;
	HudHealthBar* m_hudHealthBar;
	HudMissionMeter* m_hudMissionMeter;
	HudTutorialButtonPrompt* m_hudTutorialButtonPrompt;
	HudPosition* m_hudPosition;
	HudFrame* m_hudFrame;
	HudTimer* m_hudTimer;
	GFxValue* m_actionHandler;
	GFxValue m_unk;
	GFxValue bottomCenter;
	GFxValue topLeft;
	GFxValue topRight;
	GFxValue bottomLeft;
	GFxValue bottomRight;
	GFxValue center;
	int m_hudType;
	int m_playerId;
	unsigned int m_flags;
	GFxValue m_unk2;
	unsigned int m_unk3;
};

static_assert(sizeof(HudSplitScreen) == 0xb4);

class CarsHud {
public:
	char unknown[0x3C];
	CMessageOwner m_messageOwner;
	Flash::Movie* m_movie;
	HudSplitScreen* hud[4];
	HudCountdown* m_hudCountdown;
	HudTimer* m_hudTimer;
	HudPosition* m_hudPosition;
	char unknown2[0x130];
	int m_playerCount;
public:
	inline void TrickCallout(CActor* actor, const char* icon, const char* callout, std::uint32_t priority) {
		CarsHud_TrickCallout(this, actor, icon, callout, priority);
	}
};

static_assert(sizeof(CarsHud) == 0x19C);

inline CarsHud** g_CarsHud = reinterpret_cast<CarsHud**>(0x0192b8b4);