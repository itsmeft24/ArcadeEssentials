#pragma once
#include "../../CMessage.hpp"
#include "Gui.hpp"
#include "../GFxMovieDef.hpp"
#include "../GFxMovieView.hpp"

namespace Flash {
	struct Movie {
		CMessageOwner m_messageOowner;
		Gui* m_owner;
		GFxMovieDef* m_movieDef;
		GFxMovieView* m_movieView;
	};
};