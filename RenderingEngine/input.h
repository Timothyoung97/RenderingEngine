#pragma once

#include <SDL.h>
#include <utility>

#define MOUSE_BUTTON_IDX(X) X - 1

namespace tre {
class Input {
	
	bool _toQuit = false;
	int _keyState[SDL_NUM_SCANCODES]{};
	int _mouseButtonState[5]{}; // total of 5 buttons in SDL2
	std::pair<Sint32, Sint32> _deltaDisplacement; // <xRel, yRel>
	std::pair<Sint32, Sint32> _lastPos; // <x, y>
	

public:
	Input();
	void updateInputEvent();
	bool shouldQuit();
	int getKeyState(SDL_Scancode keyIdx);
	int getMouseButtonState(int id);
	std::pair<Sint32, Sint32> getRelMouseMotion();
	
	~Input();
	Input(const Input&) = delete;
	Input& operator=(const Input&) = delete;
};
}