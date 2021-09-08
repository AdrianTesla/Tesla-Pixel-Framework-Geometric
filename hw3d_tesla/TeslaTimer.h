#pragma once
#include <chrono>

template<typename T = float>
class TeslaTimer
{
public:
	TeslaTimer() noexcept
	{
		first = std::chrono::steady_clock::now();
		last = first;
	}
	// Mark the current event and give the event span since the last Mark
	float Mark() noexcept
	{
		const auto old = last;
		last = std::chrono::steady_clock::now();
		const std::chrono::duration<T> event_span = last - old;
		return event_span.count();
	}
	// Peek how many events passed since the last Mark
	float Peek() const noexcept
	{
		return std::chrono::duration<T>(std::chrono::steady_clock::now() - last).count();
	}
	// Get the elapsed events from the timer starting
	float GetElapsedTime() const noexcept
	{
		return std::chrono::duration<T>(std::chrono::steady_clock::now() - first).count();
	}
private:
	std::chrono::steady_clock::time_point first;
	std::chrono::steady_clock::time_point last;
};