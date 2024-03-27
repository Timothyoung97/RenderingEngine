#pragma once

#include <microprofile.h>

#include <chrono>
#include <thread>

namespace tre {

class Timer {
	std::chrono::high_resolution_clock::time_point lastFrameTime;

public:
	Timer();
	float getDeltaTime();
	void spinWait();
};
}
