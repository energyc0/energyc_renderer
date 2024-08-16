#pragma once
#include <chrono>

template<typename clock = std::chrono::high_resolution_clock>
class Timer {
private:
	const typename clock::time_point _start;
	typename clock::time_point _last_processed_time;

public:
	Timer() : _start(clock::now()), _last_processed_time(_start) {}

	template<typename measure = typename std::chrono::milliseconds>
	float process_time() noexcept {
		auto time_point = clock::now();
		float delta_time = std::chrono::duration<float, typename measure::period>(time_point - _last_processed_time).count();
		_last_processed_time = time_point;
		return delta_time;
	}

	template<typename measure = std::chrono::milliseconds>
	inline float get_elapsed_time_from_start() const noexcept {
		return std::chrono::duration<float, typename measure::period>(clock::now() - _start).count();
	}

	inline auto get_start_time() const noexcept {
		return _start;
	}
};
