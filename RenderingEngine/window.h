#pragma once

#include <string>
#include <cassert>

// Using SDL
#include <SDL.h>
#include <SDL_syswm.h>

#define CHECK_SDL_ERR(condition) \
{ \
	if (!(condition)) { \
		printf("Assertion failed: %s at %s:%d\n", SDL_GetError(), __FILE__, __LINE__);	\
		assert(false); \
	} \
}

namespace tre {
class Window {

	const int width;
	const int height;
	const std::string title;
	SDL_Window* window;

public:
	HWND hwnd;

	Window(std::string title, int screen_width, int screen_height);

	void initSDLWindow();

	HWND getWindowHandle();

	~Window();
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
};
}