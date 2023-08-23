#include <stdio.h>
#include "input.h"

namespace tre {

Input::Input() {};

void Input::updateInputEvent() {

	SDL_Event e;

	SDL_PumpEvents();

	SDL_PeepEvents(&e, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);

	switch (e.type) {
	case SDL_KEYDOWN:
		if (e.key.state == SDL_PRESSED) {
			_keyState[e.key.keysym.scancode] = 1;
		}
		break;
	case SDL_KEYUP:
		if (e.key.state == SDL_RELEASED) {
			_keyState[e.key.keysym.scancode] = 0;
		}
		break;
	case SDL_MOUSEBUTTONDOWN:
		// Log Messages
		printf("Pressed button idx: %d\n", MOUSE_BUTTON_IDX(e.button.button));
		printf("Pressed button state: %d\n", _mouseButtonState[MOUSE_BUTTON_IDX(e.button.button)]);
		printf("(%d, %d)\n", _mouseRelMotion.first, _mouseRelMotion.second);
		if (e.button.state == SDL_PRESSED) {
			_mouseButtonState[MOUSE_BUTTON_IDX(e.button.button)] ^= 1;
		}
		printf("Pressed button state: %d\n", _mouseButtonState[MOUSE_BUTTON_IDX(e.button.button)]);
		break;
	case SDL_MOUSEMOTION:
		if (_mouseButtonState[MOUSE_BUTTON_IDX(SDL_BUTTON_RIGHT)]) {
			_mouseRelMotion.first = e.motion.xrel;
			_mouseRelMotion.second = e.motion.yrel;
			printf("(%d, %d)\n", _mouseRelMotion.first, _mouseRelMotion.second);
		}
		break;
	case SDL_QUIT:
		_toQuit = true;
		break;
	default:
		break;
	}
}

bool Input::shouldQuit() {
	return _toQuit;
}

int Input::getKeyState(SDL_Scancode keyIdx) {
	return _keyState[keyIdx];
}

int Input::getMouseButtonState(int button_id) {
	return _mouseButtonState[MOUSE_BUTTON_IDX(button_id)];
}

std::pair<Sint32, Sint32> Input::getRelMouseMotion() {
	return _mouseRelMotion;
}

Input::~Input() {};

}