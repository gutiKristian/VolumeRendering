#include "Application.h"

#include <GLFW/glfw3.h>

#ifdef PLATFORM_WEB
	#include <emscripten.h>
	#include <emscripten/html5.h>
	#include <emscripten/html5_webgpu.h>
#endif

#include <webgpu/webgpu_cpp.h>

int main(int argc, char* argv[])
{
	auto* app = new med::Application();
	app->Run();
#if !defined(PLATFORM_WEB)
	delete app;
#endif
	return 0;
}
