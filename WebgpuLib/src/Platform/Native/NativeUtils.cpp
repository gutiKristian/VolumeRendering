#if defined(PLATFORM_WEB)
	#error This file should only be compiled for native build
#endif

#include "Base/Utils.h"

#include <chrono>
#include <thread>

namespace base {

	void Utils::Sleep(uint32_t ms)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	}

}
