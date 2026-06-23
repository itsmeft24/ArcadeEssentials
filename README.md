<p align="center">
    <img src="https://raw.githubusercontent.com/itsmeft24/ArcadeEssentials/refs/heads/main/assets/logo.svg" width="1024"/>
</p>

---


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
- Proper UI Scaling for Non-Standard Aspect Ratios and Resolutions Above/Below 720p
- Restored Save File and Game Progression Functionality (Must be enabled in `Pentane\PluginConfig\ArcadeEssentials\settings.toml`)
- Removed Security Dongle Checks + "Maintenance Reboots" (lol, lmao even)
- Replaced Input Driver with SDL3 + Keyboard Support
- Restored Four-Player Splitscreen
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

To effectively recreate the changes bundled with ArcadeEssentials, this change log should prove helpful. Note that not every change is included in this list:
- `assets/startup.zip`
  - Replace Arcade's `all.values.aurora` file with one from either the official PC, Xbox 360, or PlayStation 3 releases, to restore the majority of the original game's physics and speed.
  - Modify Arcade's `locale.gfg` file by adding all original supported languages and maxing out the language count to 16 under the PC1 category.
  - Modify Arcade's `locale.bld` to only contain the text `PC1`
- `assets/startup3.zip`
  - Replace Arcade's `xml/GameStructure.xml` file with one from either the official PC, Xbox 360, or PlayStation 3 releases, to allow for base game missions, maps, free play options, achievements, and AI changes to appear in FrontEnd.
  - Replace Arcade's `xml/FlashButtonConfig.xml` file with a rewritten one that duplicates the `X360` column onto the other controller columns, and swaps the face buttons for the `Wii-wheel`, `Wii-nunchuk`, `Wii-classic`, and `Wii-gamecube` columns.
- `assets/xml/xml.zip`
  - Replace Arcade's entire `xml.zip` package with one from either the official PC, Xbox 360, or PlayStation 3 releases, to restore PC/Console controller button mappings.
  - Replaced `controls_wiigamecube.xml` with a modified copy of `controls_buttonaction.xml` containing the list for gamecube button mappings using `controls_buttonaction.xml`'s button naming schemes.
- `assets/ui/qrframe3.png`
  - Replace this PNG file with one that is completely transparent, to remove the QR code frame.
- `assets/ui/ui.zip`
  - Modify the `frontendicons.oct` tupperware file to include both textures and `TexturePool` entries for the `Win32Wii_Scn_ExitToWindows_IA` and `Win32Wii_Scn_ExitToWindows_AC` icons.
  - Modify the `ingameicons.oct` tupperware file to include both textures and `TexturePool` entries for the `FE_EX_Graphics_IA` and `FE_EX_Graphics_AC` icons.
  - Modify the `controllerbuttons.oct` tupperware file to include textures from PS3, Xbox 360, and Wii's `tutorialbuttons.oct` files. All button graphics minus Xbox 360 ones will have suffixes indicating their platform/controller type such as `_ps3`, `_wcc` and `_gcc`.
  - Modify `logos.zip` to contain tupperware icon files for every included language from the original game. Note: Brazilian Portuguese icon was made from scratch and not taken from Vanilla assets.
- `assets/lang/lang.zip`
  - Modify `english_ntsc_xbox360.dct` and `strings_xbox360.dct` to include the following labels, **removing empty entries as needed**:
    - `win32wii_scn_exittowindows` - Used for the Exit to Windows option in the Pause menu. (ex. `"Exit to Windows"`)
    - `fe_ex_graphics` - Used for the Graphics Settings menu label in the Options/Extras menu. (ex. `"Graphics"`)
    - `win32wii_scn_graphiclow` - Used for the Graphics Settings menu title. (ex. `"Graphics Settings"`)
    - `win32wii_scn_screenlayout` - Used for the Vertical Sync option label. (ex. `"Vertical Sync"`)
    - `win32wii_scn_item_resolution` - Used for the Fullscreen Resolution option label. (ex. `"Fullscreen Resolution"`)
    - `win32wii_scn_leftright` - Used to represent the Disabled state of the Vertical Sync option. (ex. `"Disabled"`)
    - `win32wii_scn_updown` - Used to represent the Enabled state of the Vertical Sync option. (ex. `"Enabled"`)
    - `win32wii_msg_title` - Used for the title of the popup message displayed when Graphics settings are modified. (ex. `"Settings Changed!"`)
    - `win32wii_msg_applysettings` - Used for the body of the popup message displayed when Graphics settings are modified. (ex. `"Graphics settings have been saved. Please restart the game to apply your changes."`)
    - `win32wii_msg_applysettings_windowed` - Used for the body of the popup message displayed when Graphics settings are modified while the game is running in windowed mode. (ex. `"Graphics settings have been saved. Please disable windowed mode, and restart the game to apply your changes."`)
	- `arcadeessentials_motionblur` - Used for the Motion Blur option label. (ex. `"Motion Blur"`)
	- `win32wii_scn_graphicquality` - Used for the Shadow Quality option label. (ex. `"Shadow Quality"`)
	- `win32wii_scn_level_ultra` - Used to represent the Ultra state of the Shadow Quality option. (ex. `"Ultra"`)
	- `win32wii_scn_level_veryhigh` - Used to represent the Very High state of the Shadow Quality option. (ex. `"Very High"`)
	- `win32wii_scn_level_high` - Used to represent the High state of the Shadow Quality option. (ex. `"High"`)
	- `win32wii_scn_level_medium` - Used to represent the Medium state of the Shadow Quality option. (ex. `"Medium"`)
	- `win32wii_scn_level_low` - Used to represent the Low state of the Shadow Quality option. (ex. `"Low"`)
    - Translated all AE-specific labels to all 15 languages.
    - Copied every instance of a button icon label and appended `_ps3`, `_wcc` and `_gcc` to them to ensure every button glyph from `controllerbuttons.oct` would load depending on the connected controller type.
- `assets/langselectnames/langselectnames.zip`
    - Renamed `langnames_pc1.langnames` and changed to `langnames_xbox360_pc1.langnames` and modified it to contain a list of every single language in the original versions of the game.
- `assets/flash/flash.zip`
  - Replace Arcade's entire `flash.zip` package with one from either the Xbox 360 or PlayStation 3 releases, to fix free play localization, missing Auto-Drift UI, and other minor issues.
  - Add PC's `pc_` related flash files into the main game's `flash.zip` package. Do not overwrite any existing Console files.
  - Modify the `SetGraphicSetting` ActionScript function in the `pc_graphic_setting.swf` file to remove the third else-if-block `else if (id == 2)`. This is **required** for the Graphics Settings menu to work correctly.
  - Added a `fonts_all.swf` file that contains every single one of the game's fonts in one file. This is exclusively used in the Language Select Screen so that every language preview shows properly.
  - Modified `CreateImages` function in `hud_callout_lg.swf`, `hud_energymeter.swf`, `hud_buttonprompt.swf`, `hud_tutorial_buttonprompt.swf` to properly scale icons by their original size. Now that we use higher quality button icons, this is a necessity to ensure these new icons appear as if they're the game's original size, otherwise they'll appear larger and take up more screen space.

# Disclaimer
ArcadeEssentials *does not* aim to faithfully recreate or attempt to effectively replace the original Xbox 360 or PlayStation 3 versions of Cars 2: The Video Game. Many core gameplay aspects, such as physics, UI speed, gamemode scores, story progression, etc., may significantly differ from the base game, **or may not function at all**. Additionally, ArcadeEssentials makes **absolutely no guarantees** that its featureset will not change over time, so please keep this in mind if you seek to speedrun Arcade with ArcadeEssentials over the official releases. 

# Credits
- Maintainers: [itsmeft24](https://github.com/itsmeft24), [Bluedragon](https://github.com/BluedragonMask)
- Contributors: [itsmeft24](https://github.com/itsmeft24), [Bluedragon](https://github.com/BluedragonMask), [MythicalBry](https://github.com/MythicalBry)
- Special Thanks: [maximilian](https://github.com/DJmax0955), [RiskiVR](https://github.com/RiskiVR)
