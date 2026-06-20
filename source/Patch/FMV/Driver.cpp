#include <sunset.hpp>

#include "Driver.hpp"
#include "../../Game/Renderer/Texture.hpp"
#include "../../Game/Renderer/Material.hpp"
#include "../../Game/Renderer/Context.hpp"
#include "../../Game/Renderer/Driver.hpp"
#include "../../Game/Scaleform/Wrapper/Gui.hpp"
#include "../../Game/Utils/Vector2.hpp"
#include "../../Game/AudioEventManager.hpp"

using R_SubmitJobsFn = void(_cdecl*)();

inline auto Media_StreamExists = (bool(_cdecl*)(const char*))(0x007676a0);
inline auto r_waitForRenderThread = (void(_cdecl*)())(0x008745b0);
inline auto r_reset = (void(_cdecl*)())(0x00874440);
inline auto r_renderFrame = (void(_cdecl*)())(0x008756e0);
inline auto r_setSubmitJobsFn = (R_SubmitJobsFn(_cdecl*)(R_SubmitJobsFn))(0x00874110);
inline auto _commit_bink_job = (R_SubmitJobsFn)(0x00cdb1c0);
inline auto Good_sleep_us = (int(_cdecl*)(int))(0x00cdb0e0);

inline Renderer::Material** s_BinkShader = reinterpret_cast<Renderer::Material**>(0x01925f34);
inline Renderer::Material** s_BinkMaterial = reinterpret_cast<Renderer::Material**>(0x01925ebc);

namespace Media {
    bool StreamExists(const char* path) {
        return Media_StreamExists(path);
    }
};

inline auto BinkOpenDirectSound = (void* (__stdcall**)(std::uint32_t))(0x0159170c);
inline auto BinkSetSoundSystem = (std::int32_t(__stdcall**)(void*, std::uint32_t))(0x01591708);
inline auto BinkOpen = (BINK * (__stdcall**)(const char*, std::uint32_t))(0x01591704);
// inline auto BinkGetTrackID = (int(__stdcall**)(BINK*, std::uint32_t))(0x013ff634);
inline auto BinkClose = (void(__stdcall**)(BINK*))(0x015916fc);
inline auto BinkSetSoundTrack = (void(__stdcall**)(std::uint32_t, std::uint32_t*))(0x01591700);
// inline auto BinkSetSpeakerVolumes = (void(__stdcall**)(BINK*, std::int32_t, std::uint32_t*, std::int32_t*, std::uint32_t))(0x013ff604);
inline auto BinkSetVolume = (void(__stdcall**)(BINK*, std::uint32_t, std::int32_t))(0x01591718);
inline auto BinkGetSummary = (void(__stdcall**)(BINK*, BINKSUMMARY*))(0x015916f8);
inline auto BinkGetFrameBuffersInfo = (void(__stdcall**)(BINK*, void*))(0x015916ec);
inline auto BinkRegisterFrameBuffers = (void(__stdcall**)(BINK*, void*))(0x015916e8);
inline auto BinkWait = (std::int32_t(__stdcall**)(BINK*))(0x015916e4);
inline auto BinkDoFrame = (std::int32_t(__stdcall**)(BINK*))(0x015916e0);
inline auto BinkShouldSkip = (std::int32_t(__stdcall**)(BINK*))(0x015916dc);
inline auto BinkNextFrame = (void(__stdcall**)(BINK*))(0x015916f4);
inline auto BinkPause = (std::int32_t(__stdcall**)(BINK*, std::int32_t))(0x01591710);

constexpr auto BINK_MAX_TRACKS = 6;

auto CopyPlane(void* dst, unsigned int dstPitch, void* src, unsigned int srcPitch, unsigned int height) {
	if (!dst || !src) return;
	unsigned int copyWidth = (std::min)(dstPitch, srcPitch);
	for (unsigned int y = 0; y < height; ++y) {
		memcpy(static_cast<char*>(dst) + (y * dstPitch), static_cast<char*>(src) + (y * srcPitch), copyWidth);
	}
}

DefineReplacementHook(FMVDriverPlay) {
	static bool __fastcall callback(FMV::Driver * _this, std::uintptr_t, const char* fmvName, int langId, bool* aborted, bool(*_cdecl callback)(void*), void* callbackUserData, bool unused, bool(*_cdecl arcadeCallback)()) {
		if (GetModuleHandleA("binkw32.dll") == nullptr) {
			*aborted = true;
			return false;
		}

		auto BinkGetTrackID = (int(__stdcall*)(BINK*, std::uint32_t))(GetProcAddress(GetModuleHandleA("binkw32.dll"), "_BinkGetTrackID@8"));
		auto BinkSetSpeakerVolumes = (void(__stdcall*)(BINK*, std::int32_t, std::uint32_t*, std::int32_t*, std::uint32_t))(GetProcAddress(GetModuleHandleA("binkw32.dll"), "_BinkSetSpeakerVolumes@20"));
		
		bool isVistaOrNewer = true;
		bool shouldDisableMSAA = false;
		// Renderer::Texture::MultisampleMode mode = (*Renderer::g_Driver)->m_Impl->GetMultisampleType();
		// bool shouldDisableMSAA = isVistaOrNewer && mode != Renderer::Texture::MultisampleMode::None;
		if (shouldDisableMSAA) {
			// (*Renderer::g_Driver)->m_Impl->SetMultisampleForBink(Renderer::Texture::MultisampleMode::None);
		}
		(*BinkSetSoundSystem)(*BinkOpenDirectSound, 0);
		*aborted = false;
		if (callback != nullptr) {
			*aborted = callback(callbackUserData);
			if (*aborted != false) {
				if (shouldDisableMSAA) {
					// (*Renderer::g_Driver)->m_Impl->SetMultisampleForBink(mode);
				}
				return false;
			}
		}
		std::string path = std::format("data_dx\\fmv\\{}.bik", fmvName);
		int volume = static_cast<int>((*g_AudioEventManager)->GetVolume(AudioEventManager::Category::Sfx) * 32767.0);
		_this->m_hBink = (*BinkOpen)(path.data(), 0);
		if (_this->m_hBink == nullptr) {
			if (shouldDisableMSAA != false) {
				// (*Renderer::g_Driver)->m_Impl->SetMultisampleForBink(mode);
			}
			return false;
		}

		int numTracks = _this->m_hBink->NumTracks;
		unsigned int trackId[BINK_MAX_TRACKS] = {};
		for (int i = 0; i < BINK_MAX_TRACKS; i++) {
			trackId[i] = i << 16;
		}
		if (numTracks == 1 || numTracks == BINK_MAX_TRACKS) {
			langId = 0;
		}
		else {
			numTracks = BINK_MAX_TRACKS;
			int i = 0;
			while (i < _this->m_hBink->NumTracks && BinkGetTrackID(_this->m_hBink, i) != langId) {
				i++;
			}
			if (i == _this->m_hBink->NumTracks) {
				langId = 0;
			}
		}
		for (int i = 0; i < BINK_MAX_TRACKS; i++) {
			trackId[i] |= langId;
		}
		(*BinkClose)(_this->m_hBink);
		(*BinkSetSoundTrack)(numTracks, trackId);
		_this->m_hBink = (*BinkOpen)(path.data(), 0x4000);
		if (_this->m_hBink == nullptr) {
			if (shouldDisableMSAA != false) {
				// (*Renderer::g_Driver)->m_Impl->SetMultisampleForBink(mode);
			}
			return false;
		}
		
		if (numTracks == 1) {
			int speakerVolumes[6] = { volume, volume, 0, 0, 0, 0 };
			BinkSetSpeakerVolumes(_this->m_hBink, 0, 0, speakerVolumes, sizeof(speakerVolumes) / sizeof(int));
		}
		else {
			for (int i = 0; i < numTracks; i++) {
				unsigned int bin = trackId[i] >> 16;
				BinkSetSpeakerVolumes(_this->m_hBink, trackId[i], &bin, 0, 1);
			}
		}
		for (int i = 0; i < numTracks; i++) {
			(*BinkSetVolume)(_this->m_hBink, trackId[i], volume);
		}
		Vector2 offset(0, 0);
		BINKSUMMARY summary{};
		(*BinkGetSummary)(_this->m_hBink, &summary);
		Vector2 screenSize(
			static_cast<float>((*Renderer::g_Driver)->m_impl->m_configuration.m_windowWidth),
			static_cast<float>((*Renderer::g_Driver)->m_impl->m_configuration.m_windowHeight)
		);
		if (!(*Renderer::g_Driver)->m_impl->m_isWideScreen) {
			float scaleX = screenSize.x / summary.Width;
			float scaleY = screenSize.y / summary.Height;
			float scale = (std::min)(scaleX, scaleY);
			float actualWidth = summary.Width * scale;
			float actualHeight = summary.Height * scale;
			offset.x = (screenSize.x - actualWidth) / 2.0;
			offset.y = (screenSize.y - actualHeight) / 2.0;
		}
		(*g_AudioEventManager)->Pause(true);

		r_waitForRenderThread();
		Types::SmartPointer<Renderer::Material> binkShader = (*s_BinkShader)->Clone();
		*s_BinkMaterial = binkShader.Get();
		r_reset();
		/*
		r_waitForRenderThread();
		Renderer::Material::Clone(s_BinkShader, &local_140);
		s_BinkMaterial = local_140;
		r_reset();
		*/
		R_SubmitJobsFn oldSubmitFn = r_setSubmitJobsFn(_commit_bink_job);

		unsigned int isMultiThreaded = (*Renderer::r_context)->flags >> 1 & 1;
		(*Renderer::r_context)->flags = (*Renderer::r_context)->flags & 0xfffffffd;
		(*BinkGetFrameBuffersInfo)(_this->m_hBink, &(_this->m_binkTextureSet).bink_buffers);
		if (_this->CreateTextures(&_this->m_binkTextureSet)) {
			if ((_this->m_binkTextureSet).bink_buffers.Frames[0].APlane.Allocate != 0) {
				binkShader->SetTechnique(binkShader->GetTechnique("UseAlphaPlane"));
			}

			for (unsigned int i = 0; i < _this->m_binkTextureSet.bink_buffers.TotalFrames; ++i) {
				auto& frame = _this->m_binkTextureSet.bink_buffers.Frames[i];

				// 1. Allocate and clear Bink's internal system RAM
				unsigned int ySize = frame.YPlane.BufferPitch * _this->m_binkTextureSet.bink_buffers.YABufferHeight;
				frame.YPlane.Buffer = malloc(ySize);
				memset(frame.YPlane.Buffer, 0, ySize); // Black Luma

				unsigned int crSize = frame.cRPlane.BufferPitch * _this->m_binkTextureSet.bink_buffers.cRcBBufferHeight;
				frame.cRPlane.Buffer = malloc(crSize);
				memset(frame.cRPlane.Buffer, 128, crSize); // Neutral Chroma

				unsigned int cbSize = frame.cBPlane.BufferPitch * _this->m_binkTextureSet.bink_buffers.cRcBBufferHeight;
				frame.cBPlane.Buffer = malloc(cbSize);
				memset(frame.cBPlane.Buffer, 128, cbSize); // Neutral Chroma

				if (frame.APlane.Allocate) {
					unsigned int aSize = frame.APlane.BufferPitch * _this->m_binkTextureSet.bink_buffers.YABufferHeight;
					frame.APlane.Buffer = malloc(aSize);
					memset(frame.APlane.Buffer, 255, aSize); // Opaque
				}

				// 2. Lock and clear the DirectX staging textures so the engine doesn't render zeroes (Green)
				unsigned int texPitch = 0;
				void* texData = nullptr;

				texData = _this->Y[i]->Lock(Renderer::LockType::OverwriteAll, &texPitch);
				memset(texData, 0, texPitch * _this->m_binkTextureSet.bink_buffers.YABufferHeight);
				_this->Y[i]->Unlock();

				texData = _this->Cr[i]->Lock(Renderer::LockType::OverwriteAll, &texPitch);
				memset(texData, 128, texPitch * _this->m_binkTextureSet.bink_buffers.cRcBBufferHeight);
				_this->Cr[i]->Unlock();

				texData = _this->Cb[i]->Lock(Renderer::LockType::OverwriteAll, &texPitch);
				memset(texData, 128, texPitch * _this->m_binkTextureSet.bink_buffers.cRcBBufferHeight);
				_this->Cb[i]->Unlock();

				if (_this->A.Count() != 0) {
					texData = _this->A[i]->Lock(Renderer::LockType::OverwriteAll, &texPitch);
					memset(texData, 255, texPitch * _this->m_binkTextureSet.bink_buffers.YABufferHeight);
					_this->A[i]->Unlock();
				}
			}

			(*BinkRegisterFrameBuffers)(_this->m_hBink, &(_this->m_binkTextureSet).bink_buffers);
			while (_this->m_hBink->FrameNum != _this->m_hBink->Frames) {
				if (callback != nullptr && !*aborted) {
					*aborted = callback(callbackUserData);
				}
				if (*aborted) {
					break;
				}
				(*g_FlashGui)->Update();
				if (_this->m_subTitles != nullptr) {
					_this->m_subTitles->Update(_this->m_hBink->FrameNum);
				}
				if (*reinterpret_cast<bool*>(0x0190632a)) {
					return false;
				}
				if ((*BinkWait)(_this->m_hBink) == 0) {
					unsigned int frameIndex = (_this->m_binkTextureSet).bink_buffers.FrameNum;
					auto& binkFrame = (_this->m_binkTextureSet).bink_buffers.Frames[frameIndex]; 
					
					(*BinkDoFrame)(_this->m_hBink);
					while ((*BinkShouldSkip)(_this->m_hBink) != 0) {
						(*BinkNextFrame)(_this->m_hBink);
						(*BinkDoFrame)(_this->m_hBink);
					}

					unsigned int texPitch = 0;
					void* texData = nullptr;
					texData = _this->Y[frameIndex]->Lock(Renderer::LockType::OverwriteAll, &texPitch);
					CopyPlane(texData, texPitch, binkFrame.YPlane.Buffer, binkFrame.YPlane.BufferPitch, _this->m_binkTextureSet.bink_buffers.YABufferHeight);
					_this->Y[frameIndex]->Unlock();
					texData = _this->Cr[frameIndex]->Lock(Renderer::LockType::OverwriteAll, &texPitch);
					CopyPlane(texData, texPitch, binkFrame.cRPlane.Buffer, binkFrame.cRPlane.BufferPitch, _this->m_binkTextureSet.bink_buffers.cRcBBufferHeight);
					_this->Cr[frameIndex]->Unlock();
					texData = _this->Cb[frameIndex]->Lock(Renderer::LockType::OverwriteAll, &texPitch);
					CopyPlane(texData, texPitch, binkFrame.cBPlane.Buffer, binkFrame.cBPlane.BufferPitch, _this->m_binkTextureSet.bink_buffers.cRcBBufferHeight);
					_this->Cb[frameIndex]->Unlock();
					if (_this->A.Count() != 0) {
						texData = _this->A[frameIndex]->Lock(Renderer::LockType::OverwriteAll, &texPitch);
						CopyPlane(texData, texPitch, binkFrame.APlane.Buffer, binkFrame.APlane.BufferPitch, _this->m_binkTextureSet.bink_buffers.YABufferHeight);
						_this->A[frameIndex]->Unlock();
					}

					binkShader->SetTexture(_this->Y[(_this->m_binkTextureSet).bink_buffers.FrameNum].Get(), "YTexture", 0);
					binkShader->SetTexture(_this->Cb[(_this->m_binkTextureSet).bink_buffers.FrameNum].Get(), "cBTexture", 0);
					binkShader->SetTexture(_this->Cr[(_this->m_binkTextureSet).bink_buffers.FrameNum].Get(), "cRTexture", 0);
					if (_this->A.Count() != 0) {
						binkShader->SetTexture(_this->A[(_this->m_binkTextureSet).bink_buffers.FrameNum].Get(), "ATexture", 0);
					}
					r_renderFrame();
					if (*reinterpret_cast<unsigned int*>(0x01925e60) == 0) {
						(*BinkPause)(_this->m_hBink, 1);
						while (*reinterpret_cast<unsigned int*>(0x01925e60) == 0) {
							(*Renderer::g_Driver)->m_impl->PumpMessages();
							Sleep(50);
						}
						(*BinkPause)(_this->m_hBink, 0);
					}
					int i = 1;
					do {
						i = (*BinkWait)(_this->m_hBink);
					} while (i != 0);

					(*BinkNextFrame)(_this->m_hBink);
				}
				else {
					Good_sleep_us(500);
				}
			}
		}
		r_reset();
		r_setSubmitJobsFn(oldSubmitFn);
		(*Renderer::r_context)->flags = (*Renderer::r_context)->flags & 0xfffffffd | (isMultiThreaded & 1) << 1;
		_this->Y.Clear();
		_this->Cb.Clear();
		_this->Cr.Clear();
		_this->A.Clear();
		(*BinkClose)(_this->m_hBink);
		_this->m_hBink = nullptr;
		(*g_AudioEventManager)->Unpause(true);
		/*
		if (GATIEyefinityData_iEnableATIEyefinity == 0) {
			RestoreDefaultTarget();
		}
		*/
		if (shouldDisableMSAA != false) {
			// (*Renderer::g_Driver)->m_Impl->SetMultisampleForBink(mode);
		}

		for (unsigned int i = 0; i < _this->m_binkTextureSet.bink_buffers.TotalFrames; ++i) {
			auto& frame = _this->m_binkTextureSet.bink_buffers.Frames[i];
			free(frame.YPlane.Buffer);
			free(frame.cRPlane.Buffer);
			free(frame.cBPlane.Buffer);
			if (frame.APlane.Allocate) free(frame.APlane.Buffer);
		}
		return true;
	}
};

auto install_fmv_driver() -> void {
	FMVDriverPlay::install_at_ptr(0x00cd9f00);
}