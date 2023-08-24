#include "timer.h"

using namespace std::chrono;

namespace tre {
Timer::Timer()
{
	lastFrameTime = high_resolution_clock::now();
}

double Timer::getDeltaTime()
{
	high_resolution_clock::time_point currentFrameTime = high_resolution_clock::now();
	duration<double, std::milli> frameTime = currentFrameTime - lastFrameTime;
	return frameTime.count();
}
}