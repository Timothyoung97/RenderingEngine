#pragma once

#include "window.h"

namespace tre {

class Window;

class Engine {
public:
	
	Window* window;

	void init();
	void close();

};
}