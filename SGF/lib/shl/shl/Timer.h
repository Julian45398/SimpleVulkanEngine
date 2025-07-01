#pragma once

#include <stdint.h>
#include <chrono>

namespace shl {
    class Timer {
    private:
        uint64_t time;
    public:
        Timer()
        {
            time = nanos();
        }
        // Get time stamp in milliseconds.
		static uint64_t millis()
		{
			uint64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::high_resolution_clock::now().time_since_epoch())
				.count();
			return ms;
		}

        // Get time stamp in microseconds.
		static uint64_t micros()
		{
			uint64_t us = std::chrono::duration_cast<std::chrono::microseconds>(
				std::chrono::high_resolution_clock::now().time_since_epoch())
				.count();
			return us;
		}

        // Get time stamp in nanoseconds.
		static uint64_t nanos()
		{
			uint64_t ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
				std::chrono::high_resolution_clock::now().time_since_epoch())
				.count();
			return ns;
		}
        void reset()
        {
            time = nanos();
        }
        uint64_t currentNanos()
        {
            uint64_t current = nanos();
            return current - time;
        }
        double currentMicros()
        {
            uint64_t current = nanos();
            return (current - time) / (double)1.0E3;
        }
        double currentMillis()
        {
            uint64_t current = nanos();
            return (current - time) / (double)1.0E6;
        }
        double currentSeconds()
        {
            uint64_t current = nanos();
            return (current - time) / (double)1.0E9;
        }
        uint64_t ellapsedNanos()
        {
            uint64_t current = nanos();
            uint64_t ellapsed = current - time;
            time = current;
            return ellapsed;
        }
        double ellapsedMicros()
        {
            return (double)ellapsedNanos() / (double)1.0E3;
        }
        double ellapsedMillis()
        {
            return (double)ellapsedNanos() / (double)1.0E6;
        }
        double ellapsedSeconds()
        {
            return (double)ellapsedNanos() / (double)1.0E9;
        }
    };
}

