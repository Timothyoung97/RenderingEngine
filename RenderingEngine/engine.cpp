#include "engine.h"

namespace tre {

void Engine::init() {
	window = new Window{ "RenderingEngine", tre::SCREEN_WIDTH, tre::SCREEN_HEIGHT };
}

void Engine::close() {
	delete window;
}

}