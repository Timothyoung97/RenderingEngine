#pragma once

#include <SDL.h>

namespace tre {
class Input {
	
	bool _toQuit = false;
	int _keyState[SDL_NUM_SCANCODES]{};

public:
	Input();
	void updateInputEvent();
	bool shouldQuit();
	int getKeyState(SDL_Scancode keyIdx);
	
	~Input();
	Input(const Input&) = delete;
	Input& operator=(const Input&) = delete;
};
}