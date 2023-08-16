#include "my_timer.h"

MyTimer::MyTimer(int fps) 
{
	targetFrameRate = fps;
	targetFrameTime = 1000.0 / targetFrameRate;
	deltaTime = 0.0;
	lastFrameTime = std::chrono::high_resolution_clock::now();
}

void MyTimer::checkForSleep()
{
	std::chrono::high_resolution_clock::time_point currentFrameTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> frameTime = currentFrameTime - lastFrameTime;
	deltaTime = frameTime.count();
	lastFrameTime = currentFrameTime;
	if (deltaTime < targetFrameTime) {
		std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<long>(targetFrameTime - deltaTime)));
	}
}
