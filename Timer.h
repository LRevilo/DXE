#pragma once
#include "Windows.h"
#include <algorithm> // std::clamp
#include <thread>  // For std::this_thread::sleep_for
#include <chrono>  // For std::chrono::duration

namespace DXE
{
	struct Timer {
		LONGLONG startPerfCount = 0;
		//The frequency value indicates the number of performance counter ticks per second. 
		//For instance, if the frequency is 1,000,000, the performance counter increments once every microsecond.
		LONGLONG perfCounterFrequency = 0;

		double currentTimeInSeconds = 0.0;
		double dt = 0.0;

		operator float() { return (float)dt; }

		Timer() {
			LARGE_INTEGER perfCount;
			QueryPerformanceCounter(&perfCount);
			startPerfCount = perfCount.QuadPart;

			LARGE_INTEGER perfFreq;
			QueryPerformanceFrequency(&perfFreq);
			perfCounterFrequency = perfFreq.QuadPart;
		}

		void Tick() {
			double previousTimeInSeconds = currentTimeInSeconds;
			LARGE_INTEGER perfCount;
			QueryPerformanceCounter(&perfCount);

			currentTimeInSeconds = (double)(perfCount.QuadPart - startPerfCount) / (double)perfCounterFrequency;
			dt = (double)(currentTimeInSeconds - previousTimeInSeconds);
			dt = std::clamp(dt, 1.0 / 6000.0, 1.0 / 30.0);

		}
		void SleepUntil(double targetFrameTime) const {

			double targetTime = currentTimeInSeconds + targetFrameTime;
			while (true) {
				LARGE_INTEGER perfCount;
				QueryPerformanceCounter(&perfCount);

				double time = (double)(perfCount.QuadPart - startPerfCount) / (double)perfCounterFrequency;

				double remainingTime = targetTime - time;
				if (remainingTime <= 0) break; // Exit when time is up

				if (remainingTime > 0.0005) {
					// Yield CPU time when waiting for more than ~0.5ms
					std::this_thread::yield();
				}
			}

		}
	};

}