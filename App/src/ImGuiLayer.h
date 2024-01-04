#pragma once
#include "Base/GraphicsContext.h"

struct GLFWwindow;

namespace med
{
	class ImGuiLayer
	{
	public:
		static void InitializeContext(GLFWwindow* windowHandle);
		static void Begin();
		static void End(WGPUCommandEncoder encoder);
		static void Destroy();
	};
}
