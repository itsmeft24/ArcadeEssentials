#pragma once
#include "../Scaleform/Wrapper/Movie.hpp"

inline auto _Subtitles_Subtitles = (void* (__thiscall*)(void*, Flash::Movie*, bool))(0x0055b990);
inline auto _Destroy_Genie_Array_U32 = (void* (__thiscall*)(void*))(0x00f5c850);

class Subtitles {
private:
	char reserved[0x40] = {};
public:
	Subtitles(const Subtitles&) = delete;
	Subtitles& operator=(const Subtitles&) = delete;
	inline virtual void LoadingMovie(const char* fileName) {};
	inline virtual void Update(int frameNum) {};
	inline virtual void FinishedMovie() {};

	inline Subtitles(Flash::Movie* movie, bool unk) {
		_Subtitles_Subtitles(this, movie, unk);
	}
	inline ~Subtitles() {
		_Destroy_Genie_Array_U32(&reserved[8]);
	}
};