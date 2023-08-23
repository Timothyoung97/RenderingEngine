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
			keyState[e.key.keysym.scancode] = 1;
		}
		break;
	case SDL_KEYUP:
		if (e.key.state == SDL_RELEASED) {
			keyState[e.key.keysym.scancode] = 0;
		}
		break;
	case SDL_MOUSEBUTTONDOWN:
		if (e.button.state == SDL_PRESSED) {
			mouseButtonState[MOUSE_BUTTON_IDX(e.button.button)] = 1;
			// mouse pressed position
			lastPos.x = e.button.x;
			lastPos.y = e.button.y;
		}
		break;
	case SDL_MOUSEMOTION:
		if (mouseButtonState[MOUSE_BUTTON_IDX(SDL_BUTTON_RIGHT)]) {
			// update mouse delta motion
			deltaDisplacement.x = e.motion.x - lastPos.x;
			deltaDisplacement.y = e.motion.y - lastPos.y;
			lastPos.x = e.motion.x;
			lastPos.y = e.motion.y;
		}
		break;
	case SDL_MOUSEBUTTONUP:
		if (e.button.state == SDL_RELEASED) {
			mouseButtonState[MOUSE_BUTTON_IDX(e.button.button)] = 0;

			// set detla motion back to zero
			deltaDisplacement.x = 0;
			deltaDisplacement.y = 0;
			lastPos.x = 0;
			lastPos.y = 0;
		}
		break;
	case SDL_QUIT:
		toQuit = true;
		break;
	default:
		break;
	}
}

bool Input::shouldQuit() {
	return toQuit;
}

Input::~Input() {};

}