#pragma once

#include <SDL.h>
#include <DirectXMath.h>
#include <stdio.h>

#include <utility>

#include "imgui_impl_sdl2.h"

#define MOUSE_BUTTON_IDX(X) X - 1

using namespace DirectX;

namespace tre {
class Input {
	
public:
	bool _toQuit = false;
	int _keyState[SDL_NUM_SCANCODES]{};
	int _mouseButtonState[5]{}; // total of 5 buttons in SDL2
	XMFLOAT2 _deltaDisplacement = XMFLOAT2(.0f, .0f); // <xRel, yRel>
	XMFLOAT2 _lastPos = XMFLOAT2(.0f, .0f); // <x, y>
	float _mouseWheelScollY = 0;

	Input();
	void updateInputEvent();
	bool shouldQuit();
	
	~Input();
	Input(const Input&) = delete;
	Input& operator=(const Input&) = delete;
};
}