# ArcadeEssentials
A [Pentane](https://github.com/high-octane-dev/pentane) plugin for Cars 2: Arcade (v1.42) that restores a significant amount of critical base game functionality. Made with love by [itsmeft24](https://github.com/itsmeft24)!

# Features
ArcadeEssentials comes with a whole host of features and bug fixes, including (but not limited to!):
- Support for Windows 11-style Dark Mode
- Restored `FrontEnd` Menus
- Functional Audio Settings Menu
- Brand-New Graphics Settings Menu
- Restored Per-MissionMode + In-Game Console UI
- Fixed UI SRGB/Linear Color-Space Mismatch
- Proper UI Scaling for Resolutions > 720p
- Basic Save File Functionality
- Removed Security Dongle Checks + "Maintenance Reboots" (lol, lmao even)
- Replaced Input Driver with its XInput/DirectInput Equivalent
- Removed AutoPilot
- Removed `CarsCameraInfo.lua` Distance Clamping
- Removed Global FOV Multiplier
- Implemented `SpinOut` and `SideBash` Handling
- Restored Manual Backwards Driving
- Removed Global Timers (3m Race Timer/ 15s `FrontEnd` Timer)
- and many, many more!

# Downloads
Head to the [Releases](https://github.com/itsmeft24/ArcadeEssentials/releases/latest) page to get the latest build!
## Installation and Usage
Note that you **will need [Pentane](https://github.com/high-octane-dev/pentane) installed and properly configured** to install ArcadeEssentials. Full installation and setup tutorials for both *Pentane* and *ArcadeEssentials* can be found on [the high-octane-dev organization website](https://high-octane-dev.github.io/). 

## Requisite Asset Changes
Each release of ArcadeEssentials will include several asset changes to ensure the most functional game experience. It is **highly recommended** that you either install the bundled assets inside ArcadeEssentials' `release.zip`, *or* install an overhaul mod that includes these changes to obtain the most functional game experience.

To effectively recreate the changes bundled with ArcadeEssentials, this list should prove helpful: 
- `assets/startup.zip`
  - Replace Arcade's `all.values.aurora` file with one from either the official PC, Xbox 360, or PlayStation 3 releases, to restore the majority of the original game's physics and speed.
- `assets/startup3.zip`
  - Replace Arcade's `xml/GameStructure.xml` file with one from either the official PC, Xbox 360, or PlayStation 3 releases, to allow for base game missions, maps, free play options, achievements, and AI changes to appear in FrontEnd.
- `assets/xml/xml.zip`
  - Replace Arcade's entire `xml.zip` package with one from either the official PC, Xbox 360, or PlayStation 3 releases, to restore PC/Console controller button mappings.
- `assets/ui/qrframe3.png`
  - Replace this PNG file with one that is completely transparent, to remove the QR code frame.
- `assets/ui/ui.zip`
  - Modify the `frontendicons.oct` tupperware file to include both textures and `TexturePool` entries for the `Win32Wii_Scn_ExitToWindows_IA` and `Win32Wii_Scn_ExitToWindows_AC` icons.
  - Modify the `ingameicons.oct` tupperware file to include both textures and `TexturePool` entries for the `FE_EX_Graphics_IA` and `FE_EX_Graphics_AC` icons.
- `assets/lang/lang.zip`
  - Modify `english_ntsc_xbox360.dct` and `strings_xbox360.dct` to include the following labels, **removing empty entries as needed**:
    - `win32wii_scn_exittowindows` - Used for the Exit to Windows option in the Pause menu. (ex. `"Exit to Windows"`)
    - `fe_ex_graphics` - Used for the Graphics Settings menu label in the Options/Extras menu. (ex. `"Graphics"`)
    - `win32wii_scn_graphiclow` - Used for the Graphics Settings menu title. (ex. `"Graphics Settings"`)
    - `win32wii_scn_graphicquality` - Used for the Vertical Sync option label. (ex. `"Vertical Sync"`)
    - `win32wii_scn_item_resolution` - Used for the Fullscreen Resolution option label. (ex. `"Fullscreen Resolution"`)
    - `win32wii_scn_level_low` - Used to represent the Disabled state of the Vertical Sync option. (ex. `"Disabled"`)
    - `win32wii_scn_level_medium` - Used to represent the Enabled state of the Vertical Sync option. (ex. `"Enabled"`)
    - `win32wii_msg_title` - Used for the title of the popup message displayed when Graphics settings are modified. (ex. `"Settings Changed!"`)
    - `win32wii_msg_applysettings` - Used for the body of the popup message displayed when Graphics settings are modified. (ex. `"Graphics settings have been saved. Please restart the game to apply your changes."`)
    - `win32wii_msg_applysettings_windowed` - Used for the body of the popup message displayed when Graphics settings are modified while the game is running in windowed mode. (ex. `"Graphics settings have been saved. Please disable windowed mode, and restart the game to apply your changes."`)
- `assets/flash/flash.zip`
  - Replace Arcade's entire `flash.zip` package with one from either the Xbox 360 or PlayStation 3 releases, to fix free play localization, missing Auto-Drift UI, and other minor issues.
  - Add PC's `pc_` related flash files into the main game's `flash.zip` package. Do not overwrite any existing Console files.
  - Modify the `SetGraphicSetting` ActionScript function in the `pc_graphic_setting.swf` file to remove the third else-if-block `else if (id == 2)`. This is **required** for the Graphics Settings menu to work correctly.

# Disclaimer
ArcadeEssentials *does not* aim to faithfully recreate or attempt to effectively replace the original Xbox 360 or PlayStation 3 versions of Cars 2: The Video Game. Many core gameplay aspects, such as physics, UI speed, gamemode scores, story progression, etc., may significantly differ from the base game, **or may not function at all**. Additionally, ArcadeEssentials makes **absolutely no guarantees** that its featureset will not change over time, so please keep this in mind if you seek to speedrun Arcade with ArcadeEssentials over the official releases. 

# Credits
- [Me](https://github.com/itsmeft24) (wrote it lol)
- [RiskiVR](https://github.com/RiskiVR) (Testing/QA)
- [MythicalBry](https://github.com/MythicalBry) (Testing/QA)
- [maximilian](https://github.com/DJmax0955) (Testing/QA)
