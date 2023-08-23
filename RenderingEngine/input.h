#pragma once

#include <SDL.h>
#include <DirectXMath.h>
#include <utility>

#define MOUSE_BUTTON_IDX(X) X - 1

using namespace DirectX;

namespace tre {
class Input {
	
public:
	bool toQuit = false;
	int keyState[SDL_NUM_SCANCODES]{};
	int mouseButtonState[5]{}; // total of 5 buttons in SDL2
	XMFLOAT2 deltaDisplacement = XMFLOAT2(.0f, .0f); // <xRel, yRel>
	XMFLOAT2 lastPos = XMFLOAT2(.0f, .0f); // <x, y>

	Input();
	void updateInputEvent();
	bool shouldQuit();
	
	~Input();
	Input(const Input&) = delete;
	Input& operator=(const Input&) = delete;
};
}