#include "window.h"

namespace tre {

// Constructor
Window::Window(std::string title_name, int w, int h) : title(title_name), width(w), height(h) {
	initSDLWindow();
}

// Init Window
void Window::initSDLWindow() {

	CHECK_SDL_ERR(SDL_Init(SDL_INIT_VIDEO) == 0);

	window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
	CHECK_SDL_ERR(window);

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	CHECK_SDL_ERR(SDL_GetWindowWMInfo(window, &wmInfo));

	hwnd = wmInfo.info.win.window; //obtain window handler
}

HWND Window::getWindowHandle() {
	return hwnd;
}

//Destructor
void Window::cleanup() {
	SDL_DestroyWindow(window); // destroy window
	SDL_Quit(); // destroy subsystem
}
}