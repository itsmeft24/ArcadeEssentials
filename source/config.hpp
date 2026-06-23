#pragma once
#include <memory>

enum class ShadowQuality : int {
	Low = 0,
	Medium = 1,
	High = 2,
	VeryHigh = 3,
	Ultra = 4,
};

inline auto is_shadow_quality_high(ShadowQuality quality) -> bool {
	return !(quality == ShadowQuality::Low || quality == ShadowQuality::Medium);
}

inline auto resolve_shadow_quality(ShadowQuality quality) -> unsigned int {
	switch (quality) {
	case ShadowQuality::Low:
		return 512;
	case ShadowQuality::Medium:
		return 1024;
	case ShadowQuality::High:
		return 2048;
	case ShadowQuality::VeryHigh:
		return 4096;
	case ShadowQuality::Ultra:
		return 5461;
	}
	return 1024;
};

class AEConfig {
public:
	int window_width = 1280;
	int window_height = 720;
	bool vsync = true;
	ShadowQuality shadow_quality = ShadowQuality::Medium;
	bool enable_progression = false;
	bool enable_intro_fmvs = false;
	bool enable_attract_fmvs = false;
	bool enable_fast_nav = true;
	bool motion_blur = true;
public:
	AEConfig() = default;
	AEConfig(const AEConfig&) = delete;
	AEConfig& operator=(const AEConfig&) = delete;

	auto read() -> bool;
	auto finalize() const -> bool;
};

// Since we're only ever modifying this from a single thread post-boot, keeping this unlocked should be fine...
extern std::unique_ptr<AEConfig> GLOBAL_CONFIG;
