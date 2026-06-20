#pragma once
#include "../Scaleform/Wrapper/Layer.hpp"
#include "../Scaleform/Wrapper/Movie.hpp"
#include "../Scaleform/Wrapper/FlashControlMapper.hpp"
#include "../Genie/List.hpp"
#include "../Genie/Array.hpp"
#include "../Genie/String.hpp"

class FrontEndLayer : public Flash::Layer {
public:
	bool m_delete;
	bool m_isParentLayer;
	unsigned int m_unk2;
};

struct FrontEndSaveSlot {
	bool isFilled;
	char* lastPlayedLevel;
	char* timeSaved;
	char* dateSaved;
	char* spyPoints;
	char* clearanceLevelOrSlotName;
};

class MiniMenu {
public:
    Flash::Movie* movie;
};

class CarsFrontEnd;

inline CarsFrontEnd** g_FrontEnd = reinterpret_cast<CarsFrontEnd**>(0x0192b8ac);

inline auto	FrontEnd_InFrontend = (bool(__thiscall*)(FrontEndLayer*))(0x00e9dd40);

enum class CarsFrontEndScreen : int {

	// Unused. Likely for describing a `null` screen.
	Invalid = 0,

	// The fullscreen popup that appears when the user exits FrontEnd.
	ExitToTitleScreen = 1,

	// A popup that lets the user know that the game uses autosaving
	AutoSaveWarning = 2,

	// The title screen.
	TitleMenu = 3,

	// Unknown.
	Unknown = 4, // FIXME

	// The blank screen used while the initial save file is being loaded.
	SaveFileLoading = 5,

	// The menu that lets the user select a save file.
	SaveSlots = 6,

	// The Extras menu.
	MainMenu_Extras = 7,

	// The Audio Settings menu.
	Extras_Options = 8,

	// The menu that allows you to enter a cheat code.
	Extras_EnterCode = 9,

	// The menu that allows you to toggle on/off any active cheats.
	Extras_Cheats = 10,

	// The credits overlay.
	Extras_Credits = 11,

	// The main menu.
	MT_FrontEnd = 12,

	// The menu that lets you choose a Clearance Level from the C.H.R.O.M.E Missions menu.
	MainMenu_StoryMissions = 13,

	// The menu that lets you choose a mission from the preivously-selected Clearance Level.
	StoryMissions_MissionSelect = 14,

	// The Mission Brief screen that displays details about the selected story mission.
	StoryMissions_MissionDetails = 15,

	// The menu that allows you to choose which type of achievements to view.
	MainMenu_Badges = 16,

	// The screen that shows all of your owned badges.
	BadgeMenu_Badges = 17,

	// The screen that shows all of your owned crests.
	BadgeMenu_Crests = 18,

	// Unused. A screen that would have showed all of your owned suitcases.
	BadgeMenu_Suitcases = 19,

	// The menu that allows you to select a mission mode in Free Play.
	MainMenu_CustomMissions = 20,

	// The menu that allows you to select Free Play mission.
	MainMenu_MissionSelect = 21,

	// The version of MainMenu_CustomMissions that appears when you edit entries in your squad series playlist. 
	MainMenu_CustomMissions_SquadSeries = 22,

	// The version of MainMenu_MissionModeSelect that appears when you edit entries in your squad series playlist.
	MainMenu_MissionSelect_SquadSeries = 23,

	// The version of MissionSettings that appears when you edit entries in your squad series playlist. 
	MissionSettings_SquadSeries = 24,

	// The menu that displays your current Squad Series configuration.
	CustomSquadSeries = 25,

	// The menu that lets you edit individual mission settings.
	MissionSettings = 26,

	// The menu that appears when you select Garage from the Main Menu for the first time.
	GarageConnect = 27,

	// The screen that asks the user if they want to connect to World Of Cars.
	WorldOfCarsConnect = 28,

	// The menu that lets you select a character to preview in the Garage.
	MainMenu_Garage = 29,

	// The menu that appears after selecting a character to preview in the Garage.
	GarageDetails = 30,

	// The menu that displays the list of available WOCO-related missions.
	WorldOfCars_Missions = 31,

#if 1
	// The menu that appears when a LAN host is waiting for a client to exit the attract menus. Added by Raw Thrills.
	Arcade_WaitingForChallengers = 32,

	// Unknown. Added by Raw Thrills.
	Arcade_Unknown2 = 33,

	// Unknown. Added by Raw Thrills.
	Arcade_Unknown3 = 34,

	// The character selection screen.
	CarSelect = 35,
#else
	// The character selection screen.
	CarSelect = 32,

	// The Win32_Wii version's Graphics Settings menu. (Not applicable to Arcade!)
	Win32Wii_GraphicsSettings = 33,

	// The Win32_Wii version's Audio Settings menu. (Not applicable to Arcade!)
	Win32Wii_AudioSettings = 34,

	// The Win32_Wii version's Control Settings menu. (Not applicable to Arcade!)
	Win32Wii_ControlSettings = 35,

	// The Win32_Wii version's Gamepad Settings menu. (Not applicable to Arcade!)
	Win32Wii_GamepadSettings = 36,

	// The Win32_Wii version's Keyboard Settings menu. (Not applicable to Arcade!)
	Win32Wii_KeyboardSettings = 37,

	// The Win32_Wii version's Switch to Controller popup. (Not applicable to Arcade!)
	Win32Wii_SwitchController = 38,
#endif
	Axel_Online = 39,
	Axel_LobbyCreateOptions = 40,
	Axel_LobbyJoin = 41,
};

class CarsFrontEnd : public FrontEndLayer {
public:
    struct FrontendFlashFunctions {
        char reserved[0x2d8];
    };
    struct UnkFEStruct {
        char reserved[0x14];
    };
public:
    char unknown3[0x10];
    int m_uiPlatformSpin;
    char unknown4[0x20];
    unsigned int m_numFilledSaveSlots;
    FrontEndSaveSlot saveSlots[3];
    unsigned char field25_0x94;
    unsigned char field26_0x95;
    unsigned char field27_0x96;
    unsigned char field28_0x97;
    float field29_0x98[4];
    enum CarsFrontEndScreen currentScreen;
    int clearance_mission_index_2;
    int unk_game_mode_index;
    Flash::FlashControlMapper* guiController;
    Flash::Movie* field34_0xb8;
    Flash::Movie* field35_0xbc;
    unsigned char field36_0xc0;
    unsigned char field37_0xc1;
    unsigned char field38_0xc2;
    unsigned char field39_0xc3;
    unsigned int field40_0xc4;
    Flash::EngineTextureSet* controllerButtons;
    Flash::EngineTextureSet* badgeIcons;
    Flash::EngineTextureSet* frontendIcons;
    Flash::EngineTextureSet* carIcons;
    Flash::EngineTextureSet* worlds;
    Flash::EngineTextureSet* gameLogo;
    Flash::EngineTextureSet* worldOfCarsIcons;
    MiniMenu* miniMenu;
    Genie::Array<float> field49_0xe8;
    Genie::Array<float> field50_0xfc;
    FrontendFlashFunctions frontendFlashFunctions;
    Genie::List<Genie::String> field52_0x3e8;
    char* field53_0x408;
    char* previous_screen_to_set;
    char* field55_0x410;
    char* field56_0x414;
    unsigned int field57_0x418;
    char* field58_0x41c;
    unsigned int field59_0x420;
    char clearanceMissionIndex;
    unsigned char field61_0x425;
    unsigned char field62_0x426;
    unsigned char field63_0x427;
    unsigned char field64_0x428;
    unsigned char field65_0x429;
    unsigned char field66_0x42a;
    unsigned char field67_0x42b;
    char* freeplaySettingDescriptionsString;
    char* field69_0x430;
    char* freeplaySettingOptionsString;
    char* freeplaySettingDescriptions[6];
    char* freeplaySettingOptionsList[6];
    unsigned char field73_0x468[8];
    int field81_0x470;
    unsigned char field82_0x474[68];
    UnkFEStruct field150_0x4b8[11];
    unsigned int field151_0x594;
    char* field152_0x598;
    unsigned char field153_0x59c[516];
    float attractTimer;
    float field670_0x7a4;
    float field671_0x7a8;
    unsigned char field672_0x7ac;
    unsigned char field673_0x7ad;
    unsigned char field674_0x7ae;
    unsigned char field675_0x7af;
    float field676_0x7b0;
    unsigned int field677_0x7b4;
    unsigned char field678_0x7b8;
    unsigned char field679_0x7b9;
    unsigned char field680_0x7ba;
    unsigned char field681_0x7bb;
    unsigned char shouldReleaseMiniMenu;
    unsigned char field683_0x7bd;
    unsigned char field684_0x7be;
    unsigned char field685_0x7bf;
    CMessageOwner messageOwner;
    unsigned int field687_0x7cc;
    unsigned char field688_0x7d0;
    unsigned char field689_0x7d1;
    unsigned char field690_0x7d2;
    unsigned char field691_0x7d3;
    float field692_0x7d4;
    float unlockEverythingTimer;
    unsigned int controllerIndex[8];
    unsigned char field695_0x7fc;
    unsigned char field696_0x7fd;
    unsigned char field697_0x7fe;
    unsigned char field698_0x7ff;
    unsigned int field699_0x800;
    unsigned int field700_0x804;
    unsigned char field701_0x808;
    unsigned char field702_0x809;
    unsigned char field703_0x80a;
    unsigned char field704_0x80b;
    GenericMessageHandler<CarsFrontEnd> m_resetSaveDataHandler;
    GenericMessageHandler<CarsFrontEnd> m_toggleFrontEndCarsHadler;
    void** field709_0x83c;
    unsigned int field710_0x840;
    void** field711_0x844;
    unsigned int field712_0x848;
    GenericMessageHandler<CarsFrontEnd> m_dlcLoadedHandler;
    GenericMessageHandler<CarsFrontEnd> m_persistentDataResetToDefaultsHandler;
    GenericMessageHandler<CarsFrontEnd> m_showControllerPullHandler;
    unsigned char field719_0x894[52];
    bool isClearanceMission;
};

static_assert(sizeof(CarsFrontEnd) == 0x8cc);