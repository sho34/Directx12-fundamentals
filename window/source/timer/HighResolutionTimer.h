#pragma once
#include <chrono>

class hight_resolution_timer
{
private:
	static uint64_t								frame_counter;
	static double								elapsed_seconds;
	static std::chrono::high_resolution_clock	clock;

	// memory buffer to store the fps string.
	char	buffer[500];

};