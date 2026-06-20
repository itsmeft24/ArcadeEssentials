#pragma once
#include <cstdint>
#include <cstddef>

inline auto operator_new_array = (void*(_cdecl*)(std::size_t))(0x007b1670);

inline auto FUN_00ef3a30 = (void(__thiscall*)(std::uintptr_t))(0x00ef3a30);
inline auto FUN_0060e7d0 = (void(__thiscall*)(std::uintptr_t, int, int))(0x0060e7d0);
inline auto FUN_0060ee20 = (void** (__thiscall*)(std::uintptr_t, void*, int))(0x0060ee20);
inline auto FUN_00613000 = (void* (__thiscall*)(std::uintptr_t, void*))(0x00613000);
inline auto _CarsFrontEnd_SetScreen = (void(__thiscall*)(void*, enum class CarsFrontEndScreen, const char*, bool))(0x004c1440);
inline auto _CarsFrontEnd_GoBack = (void(__thiscall*)(void*))(0x004be200);
inline auto _CarsFrontEnd_SetLevelAndUnk = (void(__thiscall*)(void*, const void*))(0x004c0780);
inline auto _CarsFrontEnd_UnkHandleTrackLengthType = (void(__thiscall*)(void*, const char*))(0x004c0800);
inline auto _CarsFrontEnd_SetGameModeIndex = (void(__thiscall*)(void*, const char*))(0x004c2dc0);
inline auto _CarsFrontEnd_UNK_004c3b70 = (void(__thiscall*)(void*))(0x004c3b70);
inline auto Flash_Movie_CallFlashFunction = (void(__cdecl*)(std::uintptr_t, const char*, ...))(0x01168690);
inline auto Flash_EngineTextureLoader_LoadTextureSet = (void* (__thiscall*)(void*, const char*, bool, int))(0x0116cd20);
inline auto CTranslator_Translate = (char* (__thiscall*)(void*, const void*, bool))(0x00cf6dd0);
inline auto UnkExcelDataBase_GetUnk = (void(__thiscall*)(void*, void*, void*))(0x0052cd70);
inline auto UnkExcelDataBase_GetUnk1 = (void(__thiscall*)(void*, void*, void*))(0x0052ce80);
inline auto UnkExcelDataBase_GetUnk2 = (void(__thiscall*)(void*, void*, void*, float*, float*))(0x0052cfe0);
inline auto UnkExcelDataBase_GetUnk3 = (void(__thiscall*)(void*, std::uint32_t, void*, void*))(0x0052af10);
inline auto PersistentData_GetGlobal = (int(__thiscall*)(void*, const char*))(0x00cf0e20);
inline auto PersistentData_SetGlobal = (void(__thiscall*)(void*, const char*, std::uint32_t))(0x00cf0e40);
inline auto PersistentData_IsSetGlobal = (bool(__thiscall*)(void*, const char*))(0x00cf0e70);
inline auto FUN_00f675d0 = (std::uint32_t(__thiscall*)(void*, const char*, std::uint32_t))(0x00f675d0);
inline auto FUN_00f66d60 = (std::uint32_t(__thiscall*)(void*, std::uint32_t))(0x00f66d60);
inline auto GameCommon_SetPlayerSfxVolume = (void(__thiscall*)(void*, float, char))(0x00eb48b0);
inline auto GameCommon_SetPlayerMusicVolume = (void(__thiscall*)(void*, float, char))(0x00eb49a0);
inline auto GameCommon_SetPlayerDialogueVolume = (void(__thiscall*)(void*, float, char))(0x00eb4a90);
inline auto FUN_0116d4d0 = (void(__thiscall*)(std::uintptr_t, std::uintptr_t, const char*))(0x0116d4d0);
inline auto CSaveGame_UNK_00ee99a0 = (void(__thiscall*)(std::uintptr_t, bool))(0x00ee99a0);
inline auto CSaveGame_UNK_00eea2e0 = (void(__thiscall*)(std::uintptr_t))(0x00eea2e0);
inline auto CSaveGame_ClearLoadedData = (void(__thiscall*)(std::uintptr_t))(0x00ee9ae0);
inline auto FUN_00ba0870 = (void(__thiscall*)(std::uintptr_t, std::uintptr_t))(0x00ba0870);
inline auto FUN_0080df70 = (void(__thiscall*)(std::uintptr_t))(0x0080df70);
inline auto CMasterTimer_GetOSTime = (std::uint32_t(_cdecl*)())(0x00770280);
inline auto operator_new = (void* (_cdecl*)(std::size_t))(0x007b1650);
inline auto HudPosition_Multi_HudPosition_Multi = (void* (__thiscall*)(void*, std::uint32_t, std::uint32_t))(0x0054b220);
inline auto FUN_004d0250 = (void(__thiscall*)(std::uintptr_t, int))(0x004d0250);
inline auto _Flash_Movie_SetLoadMovie = (void(__thiscall*)(void*, const char*, float, float))(0x011676e0);
inline auto MiniMenu_ShowJoinIn = (void(__thiscall*)(void*))(0x004b8940);
inline auto MiniMenu_HideJoinIn = (void(__thiscall*)(void*))(0x004b8910);
inline auto ErrorPopup_MarkForClose = (void* (__thiscall*)(std::uintptr_t*))(0x00e9f8d0);
inline auto StorageStateMachine_PopupCallback_FUN_00e9b3c0 = (void(__thiscall*)(std::uintptr_t))(0x00e9b3c0);

inline auto	CarsPlayers_GetCamera = (void* (__thiscall*)(struct Players*, int, int))(0x0055ab60);
inline auto	CarsPlayers_GetCarId = (int(__thiscall*)(struct Players*, char*))(0x0055a820);
inline auto	CarsPlayers_UnkPipRelated = (unsigned int(__thiscall*)(struct Players*))(0x0055b5d0);
inline auto	Players_GetCameraScene = (int(__thiscall*)(std::uintptr_t, int, int))(0x00ebf7f0);
inline auto	FUN_0048ac10 = (void(_cdecl*)())(0x0048ac10);
inline auto	CarsCameraScene_FUN_00e7c340 = (void(__thiscall*)(std::uintptr_t, int))(0x00e7c340);
inline auto	FUN_00ec15e0 = (void(__thiscall*)(std::uintptr_t, int, int))(0x00ec15e0);
inline auto	FUN_00ec2110 = (void(__thiscall*)(std::uintptr_t, int))(0x00ec2110);
inline auto	SetTypeAndInitSplitScreen = (void(_cdecl*)(int))(0x00ccab10);
inline auto	GameEnv_GetWorldData = (void** (__thiscall*)(std::uintptr_t, unsigned int*, const char*, int))(0x00ebe010);
inline auto	GameEnv_SetWorldData = (void(__thiscall*)(std::uintptr_t, const char*, const char*, int))(0x00ebe060);
inline auto	SFD_Entry_GetValue = (char* (__thiscall*)(void*, int))(0x00cd02b0);
inline auto	Unk_EnsureCamera = (void* (_cdecl*)(void*))(0x004893f0);
inline auto	CameraScene_Push = (void(__thiscall*)(void*, unsigned int, unsigned int))(0x00b94d30);
inline auto	CameraScene_SetActive = (void(__thiscall*)(void*, bool))(0x00b94350);

inline auto	Players_CreatePlayerSceneDetails = (void(__thiscall*)(struct Players*, int))(0x00ebf560);
inline auto	Players_SetAvatar = (void(__thiscall*)(struct Players*, int, void*))(0x00ec15e0);
inline auto	Players_Player_SetAvatar = (void(__thiscall*)(struct Player*, void*))(0x00ec2110);
inline auto	r_defaultTarget = (struct R_Target* (_cdecl*)())(0x0087bf70);

inline auto	ChoreoTest_SetChoreo = (void(__thiscall*)(std::uintptr_t, int))(0x00b3f5f0);
inline auto	CarsHud_FUN_00551c60 = (std::uint32_t(__thiscall*)(std::uintptr_t, int))(0x00551c60);
inline auto	FlipEnglishWords = (void(_cdecl*)(wchar_t*, unsigned int))(0x0116b2d0);
inline auto GFxTranslator_TranslateInfo_SetResult_UTF16 = (void(__thiscall*)(void*, wchar_t*, int))(0x011d6170);
inline auto GFxTranslator_TranslateInfo_SetResult_UTF8 = (void(__thiscall*)(void*, char*, int))(0x011d61f0);

inline auto	FUN_004fd1c0 = (std::uint32_t(__thiscall*)(std::uintptr_t, int))(0x004fd1c0);
inline auto	FUN_00f677b0 = (std::uint32_t(__thiscall*)(void*, int))(0x00f677b0);
inline auto	FUN_00f66ca0 = (void(__thiscall*)(void*, int, int))(0x00f66ca0);
inline auto	FUN_00f66cf0 = (void(__thiscall*)(void*, int, int, unsigned int))(0x00f66cf0);
inline auto UnkDatabaseExcel_GetModelNameForCarId = (char* (__thiscall*)(std::uintptr_t, const char*))(0x0052bbc0);
inline auto DrivingAI_cAIManager_GetPlayerPtr = (void* (__thiscall*)(void*, unsigned int))(0x0040a470);
inline auto DrivingAI_cAIManagerInput_GetPlayerInput = (void** (__thiscall*)(void*, unsigned int))(0x0040aa20);
inline auto Physics_PhysicsMgr_TeleportRigidBody = (void(__thiscall*)(std::uintptr_t, void*, void*, bool))(0x00ddbe80);
inline auto Players_GetAvatarFromPlayerId = (void*(_cdecl*)(int))(0x00ec36d0);

inline std::uintptr_t* g_PopupCallback = reinterpret_cast<std::uintptr_t*>(0x01929b60);
inline void** g_PersistentData = reinterpret_cast<void**>(0x01926ef8);
inline void** g_Game = reinterpret_cast<void**>(0x01929bfc);
inline std::uintptr_t* g_SaveGame = reinterpret_cast<std::uintptr_t*>(0x0192b8d0);
inline std::uintptr_t** g_ErrorPopup = reinterpret_cast<std::uintptr_t**>(0x0192b8b0);
inline std::uintptr_t* g_UnkDatabaseExcel = reinterpret_cast<std::uintptr_t*>(0x018ae110);
inline std::uintptr_t* g_GameEnv = reinterpret_cast<std::uintptr_t*>(0x01929960);
inline std::uintptr_t* g_CollisionSystem = reinterpret_cast<std::uintptr_t*>(0x01923f10);
inline std::uint32_t* g_OSFrameMS = reinterpret_cast<std::uintptr_t*>(0x01925e24);
