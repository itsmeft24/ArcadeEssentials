#pragma once

namespace FMV {

	inline auto _FMV_Player_Constructor = (void* (__thiscall*)(void*))(0x00cd7fa0);
	inline auto _FMV_Player_Destructor = (void* (__thiscall*)(void*))(0x00cd7ff0);
	inline auto _FMV_Player_Begin = (bool (__thiscall*)(void*))(0x00cd80b0);
	inline auto _FMV_Player_Queue = (bool (__thiscall*)(void*, const char*, bool, float, bool, bool))(0x00cd81d0);
	inline auto _FMV_Player_Play = (bool (__thiscall*)(void*, bool*, bool*))(0x00cd82c0);
	inline auto _FMV_Player_End = (void (__thiscall*)(void*))(0x00cd8530);

	class Player {
	private:
		char reserved[0x1c];
	public:
		inline Player() {
			_FMV_Player_Constructor(this);
		}
		inline ~Player() {
			_FMV_Player_Destructor(this);
		}
		inline bool Begin() {
			return _FMV_Player_Begin(this);
		}
		inline bool Queue(const char* path, bool allowInterruptions, float delayTime, bool abortAfterHome, bool unk) {
			return _FMV_Player_Queue(this, path, allowInterruptions, delayTime, abortAfterHome, unk);
		}
		inline bool Play(bool* aborted, bool* arcadeAborted = nullptr) {
			return _FMV_Player_Play(this, aborted, arcadeAborted);
		}
		inline void End() {
			_FMV_Player_End(this);
		}
	};
};