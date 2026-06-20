#pragma once

namespace Flash {
	class Gui;
	class Movie;

	inline auto _Flash_Gui_Initialize = (bool(__thiscall*)(Flash::Gui*, unsigned int, unsigned int))(0x01164ce0);
	inline auto _Flash_Gui_Terminate = (void(__thiscall*)(Flash::Gui*, unsigned int, bool))(0x011659b0);
	inline auto _Flash_Gui_AddMovie = (Flash::Movie* (__thiscall*)(Flash::Gui*, const char*, bool, float, float))(0x01165de0);
	inline auto _Flash_Gui_RemoveMovie = (void (__thiscall*)(Flash::Gui*, Flash::Movie**, bool))(0x01165f10);
	inline auto _Flash_Gui_Update = (void(__thiscall*)(Flash::Gui*))(0x001166050);

	class Gui {
	private:
		char padding[0x670];
	public:
		inline bool Initialize(unsigned int ownerFlags, unsigned int heapSize) {
			return _Flash_Gui_Initialize(this, ownerFlags, heapSize);
		}
		inline void Terminate(unsigned int ownerFlags, bool waitForUpdate) {
			_Flash_Gui_Terminate(this, ownerFlags, waitForUpdate);
		}
		inline Flash::Movie* AddMovie(const char* name, bool toHead, float x, float y) {
			return _Flash_Gui_AddMovie(this, name, toHead, x, y);
		}
		inline void RemoveMovie(Flash::Movie** movie, bool waitForUpdate) {
			_Flash_Gui_RemoveMovie(this, movie, waitForUpdate);
		}
		inline void Update() {
			_Flash_Gui_Update(this);
		}
		inline bool UsingRightToLeftFont() {
			return padding[0x16b] != 0;
		}
	};
};

inline Flash::Gui** g_FlashGui = reinterpret_cast<Flash::Gui**>(0x0192e0e4);