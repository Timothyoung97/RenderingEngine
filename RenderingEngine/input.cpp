#include "input.h"

namespace tre {

Input::Input() {};

void Input::updateInputEvent() {

	SDL_Event e;

	SDL_PumpEvents();

	SDL_PeepEvents(&e, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);

	ImGui_ImplSDL2_ProcessEvent(&e);

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
		if (e.button.state == SDL_PRESSED) {
			_mouseButtonState[MOUSE_BUTTON_IDX(e.button.button)] = 1;
			// mouse pressed position
			_lastPos.x = (float) e.button.x;
			_lastPos.y = (float) e.button.y;
		}
		break;
	case SDL_MOUSEMOTION:
		if (_mouseButtonState[MOUSE_BUTTON_IDX(SDL_BUTTON_RIGHT)]) {
			// update mouse delta motion
			_deltaDisplacement.x = (float) e.motion.x - _lastPos.x;
			_deltaDisplacement.y = (float) e.motion.y - _lastPos.y;
			_lastPos.x = (float) e.motion.x;
			_lastPos.y = (float) e.motion.y;
		}
		break;
	case SDL_MOUSEBUTTONUP:
		if (e.button.state == SDL_RELEASED) {
			_mouseButtonState[MOUSE_BUTTON_IDX(e.button.button)] = 0;

			// set detla motion back to zero
			_deltaDisplacement.x = 0;
			_deltaDisplacement.y = 0;
			_lastPos.x = 0;
			_lastPos.y = 0;
		}
		break;
	case SDL_MOUSEWHEEL:
		if (_mouseButtonState[MOUSE_BUTTON_IDX(SDL_BUTTON_RIGHT)]) {
			if (e.wheel.y != 0) {
				_mouseWheelScollY = e.wheel.preciseY * .001f; // hardcoded multipler
			}
		}
		break;
	case SDL_QUIT:
		_toQuit = true;
		break;
	default:
		_mouseWheelScollY = 0;
		break;
	}
}

bool Input::shouldQuit() {
	return _toQuit;
}

Input::~Input() {};

}