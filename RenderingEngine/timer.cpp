#include "timer.h"

namespace tre {
Timer::Timer()
{
	lastFrameTime = std::chrono::high_resolution_clock::now();
}

double Timer::getDeltaTime()
{
	std::chrono::high_resolution_clock::time_point currentFrameTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> frameTime = currentFrameTime - lastFrameTime;
	return frameTime.count();
}
}