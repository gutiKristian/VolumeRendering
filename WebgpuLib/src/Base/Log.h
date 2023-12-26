#pragma once

#include "Base/Base.h"
#include "Base/Logger.h"

#include <memory>

namespace base {

	class Log
	{
	public:
		static std::shared_ptr<Logger>& GetLogger() { return s_Logger; }
		static void Init();
	private:
		static std::shared_ptr<Logger> s_Logger;
	};

}

#define LOG_TRACE(...)         ::base::Log::GetLogger()->Trace(__VA_ARGS__)
#define LOG_INFO(...)          ::base::Log::GetLogger()->Info(__VA_ARGS__)
#define LOG_WARN(...)          ::base::Log::GetLogger()->Warn(__VA_ARGS__)
#define LOG_ERROR(...)         ::base::Log::GetLogger()->Error(__VA_ARGS__)
#define LOG_CRITICAL(...)      ::base::Log::GetLogger()->Critical(__VA_ARGS__)
