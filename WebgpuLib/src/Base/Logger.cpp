#include "Logger.h"

#include <iostream>
#include <iomanip>
#include <chrono>
#include <sstream>

namespace Base {

	const char* Logger::RESET_CLR = "\x1B[0m";
	const char* Logger::RED_CLR = "\x1B[31m";
	const char* Logger::GREEN_CLR = "\x1B[32m";
	const char* Logger::YELLOW_CLR = "\x1B[33m";
	const char* Logger::RED_BOLD_CLR = "\x1B[31m\u001b[7m";
	const char* Logger::GRAY_CLR = "\x1B[90m";

	std::string Logger::GetTime()
	{
		auto now = std::chrono::system_clock::now();
		auto time = std::chrono::system_clock::to_time_t(now);

		std::stringstream ss;
		ss << std::put_time(std::localtime(&time), "%H:%M:%S");
		return ss.str();
	}

}
