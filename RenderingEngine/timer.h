#pragma once

#include <chrono>
#include <thread>

class Timer {
public:
	Timer();
	double getDeltaTime();

	~Timer() {};
private:
	std::chrono::high_resolution_clock::time_point lastFrameTime;
};