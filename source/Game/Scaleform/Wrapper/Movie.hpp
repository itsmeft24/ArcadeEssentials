#pragma once
#include "../../CMessage.hpp"
#include "Gui.hpp"
#include "../GFxMovieDef.hpp"
#include "../GFxMovieView.hpp"

inline auto Flash_Movie_SetImageLoader = (void(__thiscall*)(void*, void*))(0x01168bc0);
inline auto Flash_Movie_HandleInput = (bool(__thiscall*)(void*, bool, int))(0x01168be0);

namespace Flash {
	class Movie {
	public:
		CMessageOwner m_messageOowner;
		Gui* m_owner;
		GFxMovieDef* m_movieDef;
		GFxMovieView* m_movieView;
	public:
		inline void SetImageLoader(void* imageLoader) {
			Flash_Movie_SetImageLoader(this, imageLoader);
		}
		inline bool HandleInput(bool doLockedPlayerInput, int forceID) {
			return Flash_Movie_HandleInput(this, doLockedPlayerInput, forceID);
		}
	};
};