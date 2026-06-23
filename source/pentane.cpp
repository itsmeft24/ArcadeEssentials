#include "pentane.hpp"

void(*Pentane_LogUTF8)(PentaneCStringView*) = nullptr;
int(*Pentane_IsWindowedModeEnabled)() = nullptr;

extern "C" constexpr PluginInformation Pentane_PluginInformation = PluginInformation {
	.name = "Arcade Essentials",
	.author = "itsmeft24",
	.uuid = { 0x00, 0xFA, 0x94, 0x8B, 0x24, 0xA7, 0x4B, 0x2E, 0xA6, 0xA6, 0x18, 0xE3, 0xA1, 0x02, 0x9A, 0xB5 },
	.version = SemVer{ 3, 0, 1 },
	.minimum_pentane_version = SemVer{ 1, 0, 0 },
	.reserved = {},
};
extern "C" constexpr int Pentane_PluginDependencyCount = 0;
extern "C" constexpr PentaneUUID* Pentane_PluginDependencies = nullptr;
