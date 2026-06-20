#pragma once
#include <sunset.hpp>
#include <algorithm>
#include "../Game/Genie/List.hpp"
#include "../Game/Scaleform/GFxValue.hpp"
#include "../Game/Stage/StageEntity.hpp"
#include "../Game/CarManager.hpp"
#include "../Game/MiscOffsets.hpp"
#include "Input/WindowsSystemInputDriver.hpp"
#include "../pentane.hpp"
#include "../Axel/Axel.hpp"

std::uintptr_t __declspec(naked) return_address() {
	_asm {
		mov eax, ebp
		add eax, 4
		mov eax, dword ptr[eax]
		ret
	}
}

struct R_Viewport {
	short x;
	short y;
	short w;
	short h;
	float zNear;
	float zFar;
	float aspectX;
	float aspectY;
};

struct R_Target {
	char padding[0x5c];
	R_Viewport viewport;
};

static_assert(sizeof(R_Target) == 0x74);

struct Player {
	void* m_pScene;
	unsigned int m_PlayerId;
	void* m_Avatar;
	unsigned int unknown[9];
};

static_assert(sizeof(Player) == 0x30);

struct Players {
	void** vt;
	int m_StartPositionGroup;
	int m_StartNumPlayers;
	int m_field0xC;
	int m_field0x10;
	int m_NumPlayers;
	int m_NumLocalPlayers;
	bool m_IsPlayerDropInEnabled;
	bool m_IsSwappingEnabled;
	Genie::List<const Genie::String> m_SharedScreenLock;
	int m_Unknown[22];
	Player* m_Players;
	bool m_IsVertical;
	void* unknown3;
};

static_assert(sizeof(Players) == 0xA4);

inline Players** g_Players = reinterpret_cast<Players**>(0x0192b8bc);

const char PLAYER_COUNT_1[] = "PlayerCount1";
const char PLAYER_COUNT_4[] = "PlayerCount4";

int get_locked_player_count() {
	int count = 0;
	for (std::size_t i = 0; i < 4; i++) {
		if ((*g_InputPtr)->ControllerLocked(i)) {
			count += 1;
		}
	}
	return std::clamp(count, 1, 4);
}

DefineInlineHook(MPCarSelectUI) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
#ifdef MP_STRATEGY_AXEL
		if (axel::online()) {
			ctx.ecx.pointer = const_cast<char*>(PLAYER_COUNT_1);
			*reinterpret_cast<const char**>(ctx.ebp.unsigned_integer - 0x14) = "axel_lobby_car_select";
		}
		else {
			int locked_controllers = get_locked_player_count();
			if (locked_controllers > 1) {
				ctx.ecx.pointer = const_cast<char*>(PLAYER_COUNT_4);
				*reinterpret_cast<const char**>(ctx.ebp.unsigned_integer - 0x14) = "frnt_end_multi_car_select";
			}
		}
#else
		int locked_controllers = get_locked_player_count();
		if (locked_controllers > 1) {
			ctx.ecx.pointer = const_cast<char*>(PLAYER_COUNT_4);
			*reinterpret_cast<const char**>(ctx.ebp.unsigned_integer - 0x14) = "frnt_end_multi_car_select";
		}
#endif
	}
};

DefineReplacementHook(NetPlayer_PlayerCount) {
	static int _cdecl callback() {
#ifdef MP_STRATEGY_AXEL
		if (axel::online()) {
			return axel::player_count();
		}
		else {
			return get_locked_player_count();
		}
#else
		return get_locked_player_count();
#endif
	}
};

DefineReplacementHook(PersistentDataSetPlayerCount) {
	static void __fastcall callback(std::uintptr_t ecx) {
#ifdef MP_STRATEGY_AXEL
		if (axel::online()) {
			PersistentData_SetGlobal(*g_PersistentData, "NumPlayers", axel::player_count());
			// For experimenting with 10 players.
			// PersistentData_SetGlobal(*g_PersistentData, "NumPlayers", 10);
			// axel::CONTEXT->playerCount = 10;
			PersistentData_SetGlobal(*g_PersistentData, "MultiPlayer", 1);
			PersistentData_SetGlobal(*g_PersistentData, "ScreenFormat", 1);
			return;
		}
#endif
		PersistentData_SetGlobal(*g_PersistentData, "NumPlayers", get_locked_player_count());
		PersistentData_SetGlobal(*g_PersistentData, "MultiPlayer", get_locked_player_count() > 1);
		PersistentData_SetGlobal(*g_PersistentData, "ScreenFormat", get_locked_player_count());
	};
};

// Used for parts where we want to change how the game recognizes MP.
// The original implementation just checks if NET is enabled.
DefineReplacementHook(ArcadeManager_IsPlayerCountMoreThanOne) {
	static bool __fastcall callback(void* arcade_manager) {
#ifdef MP_STRATEGY_AXEL
		if (axel::online()) {
			return axel::player_count() > 1;
		}
#endif
		return get_locked_player_count() > 1;
	}
};

DefineInlineHook(SetDefaultTeamPlayerCount) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
#ifdef MP_STRATEGY_AXEL
		if (axel::online()) {
			*reinterpret_cast<int*>(ctx.ebp.unsigned_integer - 0x10) = axel::player_count();
			return;
		}
#endif
		*reinterpret_cast<int*>(ctx.ebp.unsigned_integer - 0x10) = get_locked_player_count();
	}
};

int __fastcall FUN_82215f08(std::uintptr_t _this, std::uintptr_t edx, int param_2, int param_3) {
	int iVar1;
	int iVar2;
	int iVar3;

	iVar3 = (int)param_3;
	iVar2 = (int)param_2;
	if (iVar3 == 0) {
		return param_2;
	}
	iVar1 = *(int*)(_this + 8);
	if (iVar1 == 1) {
		if (((iVar2 == 0) && (0 < iVar3)) && (iVar3 < 4)) {
			return param_3;
		}
	}
	else if (iVar1 == 2) {
		if ((iVar3 == 1) && ((iVar2 == 0 || (iVar2 == 1)))) {
			return param_2 + 2;
		}
	}
	else if (((iVar1 == 3) && (iVar3 == 1)) && ((-1 < iVar2 && (iVar2 < 3)))) {
		return 3;
	}
	return -1;
}

void* __fastcall CarsPlayers_GetCameraScene(std::uintptr_t _this, std::uintptr_t edx, int param_2, int param_3) {
	int uVar1;
	unsigned int iVar2;

	iVar2 = *reinterpret_cast<unsigned int*>(0x01923bcc);
	if (param_3 == -1) {
		for (; iVar2 != 0; iVar2 = *(int*)(iVar2 + 0x110)) {
			if (*(int*)(iVar2 + 0xb0) == param_2) {
				return (void*)iVar2;
			}
		}
	}
	else {
		uVar1 = FUN_82215f08(_this, 0, param_2, param_3);
		if (uVar1 != -1) {
			if ((*(int*)(_this + 0x3c) == 0) || (param_3 < 1)) {
				iVar2 = Players_GetCameraScene(_this, uVar1, 0);
				return (void*)iVar2;
			}
			iVar2 = *reinterpret_cast<int*>(0x01923bcc);
			if (*reinterpret_cast<int*>(0x01923bcc) != 0) {
				do {
					if (*(int*)(iVar2 + 0xb0) == uVar1) {
						return (void*)iVar2;
					}
					iVar2 = *(int*)(iVar2 + 0x110);
				} while (iVar2 != 0);
				return nullptr;
			}
		}
	}
	return 0;
}

void __fastcall CarsCameraScene_FUN_0048b6d0(std::uintptr_t _this, std::uintptr_t edx, ActorHandle param_1) {
	unsigned int uVar1;
	unsigned int uVar2;
	unsigned int uVar3;
	unsigned int uVar4;
	unsigned int uVar6;
	unsigned int local_14;
	unsigned int local_20;
	unsigned int* puVar7;
	unsigned int* local_24;
	float* pfVar8;
	int iVar5;
	const char* local_18 = nullptr;

	CarsCameraScene_FUN_00e7c340(_this, param_1);
	if (param_1 != 0) {
		FUN_0048ac10();
		iVar5 = (*g_CarManager)->GetCarId(param_1);
		const char* cVar6 = (*g_CarManager)->GetSplitScreenType(iVar5);
		Genie_String_Constructor_WithStr(&local_18, reinterpret_cast<const char*>(cVar6));
		local_14 = PersistentData_GetGlobal(*g_PersistentData, "NumPlayers");
		if (*reinterpret_cast<unsigned int*>(_this + 0xb0) < local_14) {
			if (local_14 == 1) {
				local_20 = 0;
			}
			else if (local_14 == 2) {
				local_20 = 1;
			}
			else if (local_14 == 3) {
				if (*reinterpret_cast<unsigned int*>(_this + 0xb0) == 2) {
					local_20 = 1;
				}
				else {
					local_20 = 2;
				}
			}
			else {
				local_20 = 2;
			}
			if ((*reinterpret_cast<unsigned int*>(0x018aa808) != 0) && (local_20 < *reinterpret_cast<unsigned int*>(0x018aa80c))) {
				puVar7 = (unsigned int*)(local_20 * 0x18 + *reinterpret_cast<unsigned int*>(0x018aa808));
				uVar6 = puVar7[1];
				uVar1 = puVar7[2];
				uVar2 = puVar7[3];
				uVar3 = puVar7[4];
				uVar4 = puVar7[5];
				*reinterpret_cast<unsigned int*>(_this + 0x280) = *puVar7;
				*reinterpret_cast<unsigned int*>(_this + 0x284) = uVar6;
				*reinterpret_cast<unsigned int*>(_this + 0x288) = uVar1;
				*reinterpret_cast<unsigned int*>(_this + 0x28c) = uVar2;
				*reinterpret_cast<unsigned int*>(_this + 0x290) = uVar3;
				*reinterpret_cast<unsigned int*>(_this + 0x294) = uVar4;
			}
			local_24 = *reinterpret_cast<unsigned int**>(0x018aa810);
			while (true) {
				if (local_24 == (unsigned int*)0x0) goto LAB_00457330;
				if ((local_24[1] == local_20) &&
					(iVar5 = strcmp((char*)*local_24, local_18), iVar5 == 0)) break;
				local_24 = (unsigned int*)local_24[0x12];
			}
			pfVar8 = (float*)(local_24 + 2);
			std::memcpy(reinterpret_cast<void*>(_this + 0x298), pfVar8, 16 * 4);
		LAB_00457330:
			Genie_String_Destructor(&local_18);
			return;
		}
		Genie_String_Destructor(&local_18);
	}
	return;
}

DefineInlineHook(CheckFlashCommand) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		auto& value = *reinterpret_cast<GFxValue*>(ctx.edx.unsigned_integer);
		logger::log_format("[Flash::Command::Command] Owner is of type: {}, not an object! Aborting...", std::to_underlying(value.type));
	}
};

DefineInlineHook(LogChosenHUD) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		char* chosenHudName = *reinterpret_cast<char**>(ctx.ebp.unsigned_integer - 0xc);
		std::uintptr_t _this = *reinterpret_cast<std::uintptr_t*>(ctx.ebp.unsigned_integer - 0x1c);
		logger::log_format("[PlayerHud::SetFlagsByName] Chose Hud: {}! Flags: {:X}", chosenHudName, *reinterpret_cast<std::uint32_t*>(_this + 0xA0));
	}
};

DefineReplacementHook(Players_InitSplitScreen) {
	static void __fastcall callback(Players * _this) {
		if (_this->m_SharedScreenLock.m_nElements != 0) {
			SetTypeAndInitSplitScreen(0);
			return;
		}

		if (_this->m_NumPlayers > 4) {
			return;
		}

		if (_this->m_NumPlayers == 1) {
			SetTypeAndInitSplitScreen(0);
		}
		else if (_this->m_NumPlayers == 2) {
			if (_this->m_IsVertical) {
				SetTypeAndInitSplitScreen(2);
			}
			else {
				SetTypeAndInitSplitScreen(1);
			}
		}
		else if (_this->m_NumPlayers == 3) {
			SetTypeAndInitSplitScreen(3);
		}
		else if (_this->m_NumPlayers == 4) {
			SetTypeAndInitSplitScreen(4);
		}
	}
};

DefineReplacementHook(CarsPlayers_InitSplitScreenHook) {
	static void __fastcall callback(Players * _this) {
#ifdef MP_STRATEGY_AXEL
		if (axel::online()) {
			SetTypeAndInitSplitScreen(0);
			return;
		}
#endif
		original(_this);
	}
};

DefineReplacementHook(Players_GetInitialAvatarType) {
	static int __fastcall callback(Players * _this, std::uintptr_t edx, int playerId) {
		logger::log_format("[Players::GetInitialAvatarType] AvatarType for: {} requested!", playerId);
		auto avatarTypeStr = std::format("AvatarTypeForPlayerId{}", playerId + 1);
		if (!PersistentData_IsSetGlobal(*g_PersistentData, avatarTypeStr.data())) {
			char* evaluated = nullptr;
			if (playerId == 0) {
				unsigned int unknown = 0;
				void** entry = GameEnv_GetWorldData(*g_GameEnv, &unknown, "DefaultAvatar", -2);
				if (*entry == nullptr) {
					// return -1;
					return 0;
				}
				evaluated = SFD_Entry_GetValue(*entry, 0);
			}
			else {
				auto defaultAvatarStr = std::format("DefaultAvatar{}", playerId + 1);
				unsigned int unknown = 0;
				void** entry = GameEnv_GetWorldData(*g_GameEnv, &unknown, defaultAvatarStr.data(), -2);
				if (*entry != nullptr) {
					evaluated = SFD_Entry_GetValue(*entry, 0);
				}
			}
			if (evaluated != nullptr) {
				int carId = (*g_CarManager)->GetCarId(evaluated);
				return carId;
			}
			// return -1;
			return 0;
		}
		else {
			return PersistentData_GetGlobal(*g_PersistentData, avatarTypeStr.data());
		}
	}
};

DefineReplacementHook(CarsFrontEndSetCarInfo) {
	static void _fastcall callback(void* _this, std::uintptr_t edx, int id, const char* name) {
		int unknown = *reinterpret_cast<int*>(*g_GameEnv + 0x6180);
		if (get_locked_player_count() == 1) {
			GameEnv_SetWorldData(*g_GameEnv, "DefaultAvatar", name, unknown);
			GameEnv_SetWorldData(*g_GameEnv, "DefaultAvatar2", "", unknown);
			GameEnv_SetWorldData(*g_GameEnv, "DefaultAvatar3", "", unknown);
			GameEnv_SetWorldData(*g_GameEnv, "DefaultAvatar4", "", unknown);
		}
		else {
			switch (id) {
			case 0:
				GameEnv_SetWorldData(*g_GameEnv, "DefaultAvatar", name, unknown);
				return;
			case 1:
				GameEnv_SetWorldData(*g_GameEnv, "DefaultAvatar2", name, unknown);
				return;
			case 2:
				GameEnv_SetWorldData(*g_GameEnv, "DefaultAvatar3", name, unknown);
				return;
			case 3:
				GameEnv_SetWorldData(*g_GameEnv, "DefaultAvatar4", name, unknown);
				return;
			}
		}
	}
};

DefineInlineHook(ShowGameHudPatch) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
#ifdef MP_STRATEGY_AXEL
		if (axel::online()) {
			ctx.ecx.unsigned_integer = 1;
			return;
		}
#endif
		ctx.ecx.unsigned_integer = get_locked_player_count();
	}
};

DefineInlineHook(ShowGameHudPatch2) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		std::uintptr_t* _this = *reinterpret_cast<std::uintptr_t**>(ctx.ebp.unsigned_integer - 0x10);
		auto func = *reinterpret_cast<void(__thiscall**)(void*)>(*_this + 16);
		func(_this);
	}
};

DefineReplacementHook(DerivedCreatePlayerSceneDetails) {
	static void _fastcall callback(Players * players, std::uintptr_t edx, int playerId) {
		if (players->m_StartNumPlayers < 4) {
			void* cameraScene = Unk_EnsureCamera(CarsPlayers_GetCameraScene(reinterpret_cast<std::uintptr_t>(players), 0, playerId, 0));
			if (cameraScene == nullptr) {
				return;
			}
			if (players->m_StartNumPlayers <= playerId) {
				unsigned int unk = CarsPlayers_UnkPipRelated(players);
				CameraScene_Push(cameraScene, unk, 0);
				CameraScene_SetActive(cameraScene, false);
				return;
			}
		}
		Players_CreatePlayerSceneDetails(players, playerId);
	}
};

DefineInlineHook(BaseCreatePlayerSceneDetails) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
#ifdef MP_STRATEGY_AXEL
		if (axel::online()) {
			ctx.ecx.unsigned_integer = static_cast<unsigned int>(*reinterpret_cast<int*>(ctx.ebp.unsigned_integer + 8) < 1);
			return;
		}
#endif
		ctx.ecx.unsigned_integer = static_cast<unsigned int>(*reinterpret_cast<int*>(ctx.ebp.unsigned_integer + 8) < (*g_Players)->m_StartNumPlayers);
	}
};

DefineInlineHook(CarsFrontEndSelectCar) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		*reinterpret_cast<int*>(ctx.esp.unsigned_integer) = *reinterpret_cast<int*>(ctx.ebp.unsigned_integer + 8);
	}
};

/*
DefineReplacementHook(LockSharedScreenHook) {
	static void _fastcall callback(Players * _this, std::uintptr_t edx, const char* str) {
		// logger::log_format("[Players::LockSharedScreen] Locking Shared Screen: {}, m_nElements: {}", str, _this->m_SharedScreenLock.m_nElements);
		original(_this, edx, str);
	}
};

DefineReplacementHook(UnlockSharedScreenHook) {
	static void _fastcall callback(Players * _this, std::uintptr_t edx, const char* str) {
		// logger::log_format("[Players::UnlockSharedScreen] Unlocking Shared Screen: {}, m_nElements: {}", str, _this->m_SharedScreenLock.m_nElements);
		original(_this, edx, str);
	}
};
*/

DefineReplacementHook(CheckChange) {
	static void _fastcall callback(void* _this, std::uintptr_t edx, const char* label, std::uint32_t value) {
		std::string_view name(label);
		if (name == "NumPlayers" || name == "MultiPlayer" || name == "ScreenFormat") {
			// logger::log_format("[PersistentData::SetGlobal] Key: {}, Value: {}, RetAddr: 0x{:X}", name, value, return_address());
		}
		original(_this, edx, label, value);
	};
};

DefineReplacementHook(CheckGet) {
	static std::uint32_t _fastcall callback(void* _this, std::uintptr_t edx, const char* label) {
		std::string_view name(label);
		auto ret = original(_this, edx, label);
		if (name == "NumPlayers" || name == "MultiPlayer" || name == "ScreenFormat") {
			// logger::log_format("[PersistentData::GetGlobal] Key: {}, Value: {}, RetAddr: 0x{:X}", name, ret, return_address());
		}
		return ret;
	};
};

/*
DefineReplacementHook(ObserveScreenCount) {
	static void _cdecl callback(struct R_Viewport* vp, int numScreens) {
		// logger::log_format("[r_updateSplitScreens] Count: {}", numScreens);
		original(vp, numScreens);
	}
};
*/

DefineReplacementHook(SetSplitScreenVertical) {
	static void __fastcall callback(Players * _this, std::uintptr_t edx, bool vertical) {
		if (vertical == _this->m_IsVertical) {
			return;
		}
		_this->m_IsVertical = vertical;
		if (_this->m_NumPlayers != 2) {
			return;
		}
		if (_this->m_SharedScreenLock.m_nElements != 0) {
			return;
		}
		if (vertical) {
			SetTypeAndInitSplitScreen(2);
			return;
		}
		SetTypeAndInitSplitScreen(1);
		return;
	}
};

DefineInlineHook(CreatePlayerHUDMovie) {
	static void __cdecl callback(sunset::InlineCtx & ctx) {
		std::uintptr_t _this = *reinterpret_cast<std::uintptr_t*>(ctx.ebp.unsigned_integer - 0x1c);
		int playerId = *reinterpret_cast<int*>(_this + 0x9C);
		int playerCount = get_locked_player_count();

		float xOffset = 0.0f;
		float yOffset = 0.0f;

		float mainW = r_defaultTarget()->viewport.w;
		float mainH = r_defaultTarget()->viewport.h;

		if (playerId != 0) {
			if (playerCount == 2) {
				// `playerId` is guaranteed to be == 1.
				if ((*g_Players)->m_IsVertical) {
					xOffset = mainW / 2.0f;
				}
				else {
					yOffset = mainH / 2.0f;
				}
			}
			else if (playerCount == 3) {
				if (playerId == 1) {
					xOffset = mainW / 2.0f;
				}
				else {
					// `playerId` is guaranteed to be == 2.
					yOffset = mainH / 2.0f;
				}
			}
			else if (playerCount == 4) {
				if (playerId == 1) {
					xOffset = mainW / 2.0f;
				}
				else if (playerId == 2) {
					yOffset = mainH / 2.0f;
				}
				else {
					// `playerId` is guaranteed to be == 2.
					xOffset = mainW / 2.0f;
					yOffset = mainH / 2.0f;
				}
			}
		}

		logger::log_format("[PlayerHud::CreateMovie] Id: {}, Count: {}, IsVertical: {}, OffsetX: {}, OffsetY: {}", playerId, playerCount, (*g_Players)->m_IsVertical, xOffset, yOffset);
		*reinterpret_cast<float*>(ctx.ebp.unsigned_integer - 0x8) = xOffset;
		*reinterpret_cast<float*>(ctx.ebp.unsigned_integer - 0x4) = yOffset;
	}
};

DefineInlineHook(FixPlayerIdxHudPosMulti) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		std::uintptr_t _this = *reinterpret_cast<std::uintptr_t*>(ctx.ebp.unsigned_integer - 0x58);
		ctx.ecx.unsigned_integer = *reinterpret_cast<int*>(_this + 0x48);
	}
};

DefineInlineHook(FixPipCameraSceneIdx) {
	static void __cdecl callback(sunset::InlineCtx& ctx) {
		*reinterpret_cast<int*>(ctx.esp.unsigned_integer) = *reinterpret_cast<int*>(ctx.edx.unsigned_integer + 0x48);
	}
};

DefineInlineHook(SetGlobalPlayerNumberString) {
	static void __cdecl callback(sunset::InlineCtx & ctx) {
		Genie_String_Constructor_WithStr(reinterpret_cast<void*>(0x018ac620), "three");
		Genie_String_Constructor_WithStr(reinterpret_cast<void*>(0x018ac624), "four");
	}
};

DefineInlineHook(FixInGameResultsSplash) {
	static void _cdecl callback(sunset::InlineCtx & ctx) {
		std::uintptr_t g_RaceManager = *reinterpret_cast<std::uintptr_t*>(0x018ae0fc);
		int humanRacerCount = *reinterpret_cast<int*>(g_RaceManager + 0x64);
		if (1 < humanRacerCount) {
			*reinterpret_cast<int*>(ctx.ebp.unsigned_integer - 0x40) = CarsHud_FUN_00551c60(*reinterpret_cast<std::uintptr_t*>(0x0192b8b4), *reinterpret_cast<int*>(ctx.ebp.unsigned_integer + 0xC));
			if (*reinterpret_cast<int*>(ctx.ebp.unsigned_integer - 0x40) == -1) {
				*reinterpret_cast<int*>(ctx.ebp.unsigned_integer - 0x40) = 0;
			}
			std::uintptr_t _this = *reinterpret_cast<std::uintptr_t*>(ctx.ebp.unsigned_integer - 0xC4);
			FUN_004fd1c0(*reinterpret_cast<std::uintptr_t*>(_this + 4), *reinterpret_cast<int*>(ctx.ebp.unsigned_integer - 0x40));
		}
	}
};

void install_multiplayer_hooks() {
	// ObserveScreenCount::install_at_ptr(0x0062b1b0);
	CheckChange::install_at_ptr(0x00cf0e40);
	// CheckGet::install_at_ptr(0x00cf0e20);
	// LockSharedScreenHook::install_at_ptr(0x00ec19b0);
	// UnlockSharedScreenHook::install_at_ptr(0x00ec1a40);

	// Dumb hack to fix an issue where HUD elements would be placed at the previous-event's location.
	CreatePlayerHUDMovie::install_at_ptr(0x0054c5b1);

	// Uhh... I don't know what this does.
	SetSplitScreenVertical::install_at_ptr(0x00ec2000);

	// sunset::inst::nop(reinterpret_cast<void*>(0x00550d1c), 4);

	// ShowGameHudPatch2::install_at_ptr(0x00550d33);

	// Fixes an issue where UI elements would only be created for the first player.
	ShowGameHudPatch::install_at_ptr(0x00550d2a);

	// Fixes an issue where the player index would not propogate through to `CarsFrontEnd::SetCarInfo`.
	CarsFrontEndSelectCar::install_at_ptr(0x004c36ed);

	// Fixes an issue where only player 0's camera would be properly handled.
	DerivedCreatePlayerSceneDetails::install_at_ptr(0x0055ad10);

	// Fixes an issue where only player 0's camera would be set to Active.
	BaseCreatePlayerSceneDetails::install_at_ptr(0x00ebf5c3);

	// Stores the player count in PersistentData. Unfortunately, I believe this doesn't do anything so I can't trust PersistentData in other hooks to check the player count;
	// I have to use `GetPlayerCount` instead.
	PersistentDataSetPlayerCount::install_at_ptr(0x004c0ac0);

	// Allows `frnt_end_multi_car_select` to appear in FrontEnd.
	MPCarSelectUI::install_at_ptr(0x004c249c);

	// For debugging only. This should only trip when theres a mismatch between the mission-mode UI chosen by the game and the player count.
	CheckFlashCommand::install_at_ptr(0x00604c8c);

	NetPlayer_PlayerCount::install_at_ptr(0x00470980);

	// Modifies a loop in `Players::SetDefaultTeam` to iterate over the player count.
	SetDefaultTeamPlayerCount::install_at_ptr(0x00ec29cf);

	// Temporary hack to fix Avatar types being stripped out. For some reason hooking `CarsFrontEnd::SetCarInfo` is not enough.
	Players_GetInitialAvatarType::install_at_ptr(0x00ec2b60);

	// Temporary hack to fix Avatar types being stripped out.
	CarsFrontEndSetCarInfo::install_at_ptr(0x004c3180);

	// Un-stubs `CarsPlayers::InitSplitScreen`.
	sunset::utils::set_permission(reinterpret_cast<void*>(0x0055a99f), sizeof(unsigned char), sunset::utils::Perm::ExecuteReadWrite);
	*reinterpret_cast<unsigned char*>(0x0055a99f) = 0x75;

	// Conditionally re-enables the old Raw Thrills path inside `CarsPlayers::InitSplitScreen` for Axel.
	CarsPlayers_InitSplitScreenHook::install_at_ptr(0x0055a980);

	// Un-stubs `Players::InitSplitScreen`.
	Players_InitSplitScreen::install_at_ptr(0x00ec34c0);

	// Logs the chosen Hud name.
	LogChosenHUD::install_at_ptr(0x0054c574);

	// Forces Horizontal Split-Screen.
	sunset::utils::set_permission(reinterpret_cast<void*>(0x00ebefbe + 3), sizeof(int), sunset::utils::Perm::ExecuteReadWrite);
	*reinterpret_cast<int*>(0x00ebefbe + 3) = 0;

	// ArcadeManager_IsPlayerCountMoreThanOne::install_at_ptr(0x0045a450);

	// `ArcadeManager::NotWaiting` stubs. Not needed if Net is turned off.
	// sunset::inst::mov_u32(reinterpret_cast<void*>(0x00458ff0), sunset::inst::RegisterIndex::Eax, 1);
	// sunset::inst::ret(reinterpret_cast<void*>(0x00458ff5));

	// Stubs `DrivingAI::cAIManager::PreUpdate`, the function responsible for handling packets from Net.
	// sunset::inst::ret(reinterpret_cast<void*>(0x004089f0));

	// Forces you into a 'solo game' in `CameraScene::CameraScene`. Don't know why this is needed, but it is.
	sunset::inst::nop(reinterpret_cast<void**>(0x00b93c02), 6);

	sunset::inst::jmp(reinterpret_cast<void*>(0x0055aaf0), FUN_82215f08);
	sunset::inst::jmp(reinterpret_cast<void*>(0x0055aba0), CarsPlayers_GetCameraScene);
	sunset::inst::jmp(reinterpret_cast<void*>(0x0048b6d0), CarsCameraScene_FUN_0048b6d0);

	// Forces a `CarsFrontEnd` member responsible for destroying the mini menu to initialize to zero.	
	sunset::utils::set_permission(reinterpret_cast<void*>(0x004b9905 + 6), sizeof(std::uint8_t), sunset::utils::Perm::ExecuteReadWrite);
	*reinterpret_cast<std::uint8_t*>(0x004b9905 + 6) = 0;
	sunset::inst::nop(reinterpret_cast<void**>(0x004bafa3), 7);
	sunset::inst::nop(reinterpret_cast<void**>(0x004c07ee), 7);

	// Fixes an issue where PIP is disabled in 2-player splitscreen.
	sunset::utils::set_permission(reinterpret_cast<void*>(0x004ef630 + 3), sizeof(std::uint8_t), sunset::utils::Perm::ExecuteReadWrite);
	*reinterpret_cast<std::uint8_t*>(0x004ef630 + 3) = 2;
	FixPipCameraSceneIdx::install_at_ptr(0x0054b76d);
	FixPlayerIdxHudPosMulti::install_at_ptr(0x0054b6e5);

	const std::array<std::uint8_t, 7> INST_PATCH_PIP = { 0x8b, 0x45, 0xe8, 0x89, 0x45, 0xf0, 0x90 };
	sunset::utils::set_permission(reinterpret_cast<void*>(0x004ef63b), INST_PATCH_PIP.size(), sunset::utils::Perm::ExecuteReadWrite);
	std::memcpy(reinterpret_cast<void*>(0x004ef63b), INST_PATCH_PIP.data(), INST_PATCH_PIP.size());

	// Fixes an issue where `CarsHUD.swf` would be used instead of `CarsHUDMulti.swf`.
	sunset::utils::set_permission(reinterpret_cast<void*>(0x00550e0b + 3), sizeof(std::uint32_t), sunset::utils::Perm::ExecuteReadWrite);
	*reinterpret_cast<std::uint32_t*>(0x00550e0b + 3) = 2;

	static const char CARS_HUD_MULTI[] = "CarsHUDMulti";
	sunset::utils::set_permission(reinterpret_cast<void*>(0x00550dfe + 3), sizeof(std::uint32_t), sunset::utils::Perm::ExecuteReadWrite);
	*reinterpret_cast<std::uint32_t*>(0x00550dfe + 3) = reinterpret_cast<std::uintptr_t>(&CARS_HUD_MULTI);

	// Fixes an issue where the results screen would show the P1 icon for P3 and P4.
	sunset::inst::nop(reinterpret_cast<void*>(0x0156dbc1), 24);
	SetGlobalPlayerNumberString::install_at_ptr(0x0156dbc1);

	// Fixes an issue where the results page that immediately appears as the last player crosses the finish line uses P1's results for all four players.
	sunset::inst::nop(reinterpret_cast<void*>(0x004fd22a), 5);
	FixInGameResultsSplash::install_at_ptr(0x0050155b);
}
