#if defined(PLATFORM_WEB)
	#error This file should only be compiled for native build
#endif

#include "Base/Base.h"
#include "Base/Filesystem.h"

namespace base {

	void Filesystem::Init()
	{
		s_AssetsPath = ASSETS_PATH;
	}

}
