#pragma once

#include <chrono>
#include <thread>

namespace tre {

class Timer {
	std::chrono::high_resolution_clock::time_point lastFrameTime;

public:
	Timer();
	double getDeltaTime();

	~Timer() {};
};
}
