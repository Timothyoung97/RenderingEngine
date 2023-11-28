#pragma once

#include "spdlog/spdlog.h"

// Using SDL
#include <SDL.h>
#include <SDL_syswm.h>


#define CHECK_SDL_ERR(condition) \
{ \
	if (!(condition)) { \
		spdlog::error("Assertion failed: {} at {}:{}\n", SDL_GetError(), __FILE__, __LINE__);	\
		assert(false); \
	} \
}

namespace tre {

static const int SCREEN_WIDTH = 1920;
static const int SCREEN_HEIGHT = 1080;

class Window {
public:

	const int width;
	const int height;
	const std::string title;
	SDL_Window* window;
	HWND hwnd;

	Window(std::string title, int screen_width, int screen_height);

	void initSDLWindow();

	HWND getWindowHandle();

	void cleanup();
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
};
}