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
		printf("Pressed button state (Before): %d\n", _mouseButtonState[MOUSE_BUTTON_IDX(e.button.button)]);
		printf("(%d, %d) (Before)\n", _deltaDisplacement.first, _deltaDisplacement.second);
		if (e.button.state == SDL_PRESSED) {
			_mouseButtonState[MOUSE_BUTTON_IDX(e.button.button)] = 1;
			// mouse pressed position
			_lastPos.first = e.button.x;
			_lastPos.second = e.button.y;
		}
		printf("(%d, %d) (After)\n", _deltaDisplacement.first, _deltaDisplacement.second);
		printf("Pressed button state (After): %d\n\n", _mouseButtonState[MOUSE_BUTTON_IDX(e.button.button)]);
		break;
	case SDL_MOUSEMOTION:
		if (_mouseButtonState[MOUSE_BUTTON_IDX(SDL_BUTTON_RIGHT)]) {
			printf("(%d, %d)\n", _deltaDisplacement.first, _deltaDisplacement.second);

			_deltaDisplacement.first = e.motion.x - _lastPos.first;
			_deltaDisplacement.second = e.motion.y - _lastPos.second;
			_lastPos.first = e.motion.x;
			_lastPos.second = e.motion.y;
			// update mouse delta motion
		}
		break;
	case SDL_MOUSEBUTTONUP:
		printf("Released button idx: %d\n", MOUSE_BUTTON_IDX(e.button.button));
		printf("Released button state (Before): %d\n", _mouseButtonState[MOUSE_BUTTON_IDX(e.button.button)]);
		printf("(%d, %d) (Before)\n", _deltaDisplacement.first, _deltaDisplacement.second);
		if (e.button.state == SDL_RELEASED) {
			_mouseButtonState[MOUSE_BUTTON_IDX(e.button.button)] = 0;

			// set detla motion back to zero
			_deltaDisplacement.first = 0;
			_deltaDisplacement.second = 0;
			_lastPos.first = 0;
			_lastPos.second = 0;
		}
		printf("(%d, %d) (After)\n", _deltaDisplacement.first, _deltaDisplacement.second);
		printf("Released button state (After): %d\n\n", _mouseButtonState[MOUSE_BUTTON_IDX(e.button.button)]);

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
	return _deltaDisplacement;
}

Input::~Input() {};

}