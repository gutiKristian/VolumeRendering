#pragma once

#include <chrono>

namespace base {

	class Timer
	{
	public:
		Timer()
		{
			Reset();
		}

		void Reset()
		{
			m_Start = std::chrono::high_resolution_clock::now();
		}

		uint64_t ElapsedNs()
		{
			return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_Start).count();
		}

		float ElapsedMs()
		{
			return ElapsedNs() / 1'000'000.0f;
		}

		float ElapsedSeconds()
		{
			return ElapsedNs() / 1'000'000'000.0f;
		}
	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
	};

}