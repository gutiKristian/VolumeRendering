#pragma once

#include <cstdint>

namespace base {

	class Timestep
	{
	public:
		explicit Timestep(uint64_t timeNs = 0)
			: m_TimeNs(timeNs) {}

		operator uint64_t() const { return m_TimeNs; }
		operator float() const { return m_TimeNs / 1'000'000'000.0f; }

		float GetSeconds() const { return m_TimeNs / 1'000'000'000.0f; }
		float GetMilliseconds() const { return m_TimeNs / 1'000'000.0f; }
		float GetMicroseconds() const { return m_TimeNs / 1'000.0f; }
		uint64_t GetNanoseconds() const { return m_TimeNs; }
	private:
		uint64_t m_TimeNs;
	};

}
