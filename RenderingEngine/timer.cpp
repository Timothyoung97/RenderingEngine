#include "timer.h"

using namespace std::chrono;

namespace tre {
Timer::Timer()
{
	lastFrameTime = high_resolution_clock::now();
}

float Timer::getDeltaTime()
{
	high_resolution_clock::time_point currentFrameTime = high_resolution_clock::now();
	duration<float, std::milli> frameTime = currentFrameTime - lastFrameTime;
	return frameTime.count();
}
}