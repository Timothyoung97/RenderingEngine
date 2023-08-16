#pragma once

#include <chrono>
#include <thread>

class MyTimer {
public:
	MyTimer(int fps);
	void checkForSleep();
	~MyTimer() {};

private:
	int targetFrameRate;
	double targetFrameTime;
	double deltaTime;
	std::chrono::high_resolution_clock::time_point lastFrameTime;
};