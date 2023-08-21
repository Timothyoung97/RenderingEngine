#pragma once

#include <chrono>
#include <thread>

class Timer {
public:
	Timer();
	double getDeltaTime();
	void update();

	~Timer() {};
private:
	std::chrono::high_resolution_clock::time_point lastFrameTime;
};