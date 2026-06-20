#include <iostream>
#include <Windows.h>
#include <pathcch.h>
#include <shlwapi.h>
#include <toml++/toml.hpp>
#include <filesystem>

#include "config.hpp"
#include "pentane.hpp"

std::unique_ptr<AEConfig> GLOBAL_CONFIG = std::make_unique<AEConfig>();

auto get_module_location() -> std::filesystem::path {
	wchar_t game_directory[1024];
	GetModuleFileNameW(nullptr, game_directory, 1024);
	PathCchRemoveFileSpec(game_directory, 1024);
	CharLowerW(game_directory);
	return game_directory;
}

auto AEConfig::read() -> bool {
	std::filesystem::path folder = get_module_location() / "Pentane" / "PluginConfig" / "ArcadeEssentials";
	std::filesystem::path file_path = folder / "settings.toml";
	if (!std::filesystem::is_regular_file(file_path)) {
		return false;
	}

	toml::table tbl{};
	try {
		tbl = toml::parse_file(file_path.string());
	}
	catch (const toml::parse_error& err) {
		logger::log_format("[ArcadeEssentials::AEConfig::read] Failed to parse `settings.toml`! VSync will be enabled, motion blur will be enabled, the game window will attempt to match 1280x720, and game progression will be disabled.");
		return false;
	}

	if (tbl.contains("Graphics")) {
		const auto& graphics_node = tbl["Graphics"];

		if (!graphics_node.as_table()->contains("VerticalSync")) {
			logger::log_format("[ArcadeEssentials::AEConfig::read] `Graphics` section is missing `VerticalSync`! VSync will be enabled.");
		}
		else {
			vsync = graphics_node["VerticalSync"].as_boolean()->get();
		}

		if (!graphics_node.as_table()->contains("MotionBlur")) {
			logger::log_format("[ArcadeEssentials::AEConfig::read] `Graphics` section is missing `MotionBlur`! Motion blur will be enabled.");
		}
		else {
			motion_blur = graphics_node["MotionBlur"].as_boolean()->get();
		}

		if (!graphics_node.as_table()->contains("ShadowQuality")) {
			logger::log_format("[ArcadeEssentials::AEConfig::read] `Graphics` section is missing `ShadowQuality`! Real-time shadows will render at 1024x1024.");
		}
		else {
			shadow_quality = static_cast<ShadowQuality>(std::clamp(static_cast<int>(graphics_node["ShadowQuality"].as_integer()->get()), std::to_underlying(ShadowQuality::Low), std::to_underlying(ShadowQuality::Ultra)));
		}

		bool found_width = false, found_height = false;

		if (!graphics_node.as_table()->contains("WindowWidth")) {
			logger::log_format("[ArcadeEssentials::AEConfig::read] `Graphics` section is missing `WindowWidth`! The game window will attempt to match 1280x720.");
		}
		else {
			found_width = true;
			window_width = graphics_node["WindowWidth"].as_integer()->get();
		}

		if (!graphics_node.as_table()->contains("WindowHeight")) {
			logger::log_format("[ArcadeEssentials::AEConfig::read] `Graphics` section is missing `WindowHeight`! The game window will attempt to match 1280x720.");
		}
		else {
			found_height = true;
			window_height = graphics_node["WindowHeight"].as_integer()->get();
		}

		// If we found a width but not a height, or vice versa, or if the user selected a negative resolution then fall back to 720p.
		if ((found_width && !found_height) || (found_height && !found_width) || window_height < 0 || window_height < 0) {
			window_width = 1280;
			window_height = 720;
		}
	}
	else {
		logger::log_format("[ArcadeEssentials::AEConfig::read] `settings.toml` is missing the `Graphics` section! VSync will be enabled, motion blur will be enabled, and the game window will attempt to match 1280x720.");
	}

	if (tbl.contains("Extra")) {
		const auto& extra_node = tbl["Extra"];

		if (!extra_node.as_table()->contains("GameProgression")) {
			logger::log_format("[ArcadeEssentials::AEConfig::read] `Extra` section is missing `GameProgression`! Game progression will be disabled.");
		}
		else {
			enable_progression = extra_node["GameProgression"].as_boolean()->get();
		}

		if (!extra_node.as_table()->contains("EnableIntroFMVs")) {
			logger::log_format("[ArcadeEssentials::AEConfig::read] `Extra` section is missing `EnableIntroFMVs`! Intro FMVs will be disabled.");
		}
		else {
			enable_intro_fmvs = extra_node["EnableIntroFMVs"].as_boolean()->get();
		}

		if (!extra_node.as_table()->contains("EnableAttractFMVs")) {
			logger::log_format("[ArcadeEssentials::AEConfig::read] `Extra` section is missing `EnableAttractFMVs`! Attract FMVs will be disabled.");
		}
		else {
			enable_attract_fmvs = extra_node["EnableAttractFMVs"].as_boolean()->get();
		}

		if (!extra_node.as_table()->contains("EnableFastNav")) {
			logger::log_format("[ArcadeEssentials::AEConfig::read] `Extra` section is missing `EnableFastNav`! Fast menu navigation will be enabled.");
		}
		else {
			enable_fast_nav = extra_node["EnableFastNav"].as_boolean()->get();
		}
	}
	else {
		logger::log_format("[ArcadeEssentials::AEConfig::read] `settings.toml` is missing the `Extra` section! Game progression and intro/attract FMVs will be disabled.");
	}

	return true;
}

auto AEConfig::finalize() const -> bool {
	toml::table tbl{};
	toml::table* graphics = tbl.insert("Graphics", toml::table{}).first->second.as_table();
	graphics->insert("WindowWidth", window_width);
	graphics->insert("WindowHeight", window_height);
	graphics->insert("VerticalSync", vsync);
	graphics->insert("MotionBlur", motion_blur);
	graphics->insert("ShadowQuality", std::to_underlying(shadow_quality));

	toml::table* extra = tbl.insert("Extra", toml::table{}).first->second.as_table();
	extra->insert("GameProgression", enable_progression);
	extra->insert("EnableIntroFMVs", enable_intro_fmvs);
	extra->insert("EnableAttractFMVs", enable_attract_fmvs);
	extra->insert("EnableFastNav", enable_fast_nav);

	std::filesystem::path folder = get_module_location() / "Pentane" / "PluginConfig" / "ArcadeEssentials";
	if (!std::filesystem::is_directory(folder)) {
		std::filesystem::create_directories(folder);
	}
	std::filesystem::path file_path = folder / "settings.toml";
	std::ofstream out(file_path, std::ios::out);
	if (!out.is_open()) {
		return false;
	}
	out << tbl;
	out.close();
	return true;
}
