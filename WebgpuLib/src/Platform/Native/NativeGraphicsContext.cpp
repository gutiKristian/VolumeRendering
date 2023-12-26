#if defined(PLATFORM_WEB)
	#error This	file should only be compiled for native build
#endif

#include "Base/Base.h"
#include "Base/Utils.h"
#include "Base/GraphicsContext.h"

#include <webgpu/webgpu.h>
#include <webgpu/webgpu_cpp.h>

#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace base {

	static wgpu::Surface GetGLFWSurface(wgpu::Instance instance, GLFWwindow* window)
	{
		HWND hWnd = glfwGetWin32Window(window);
		HINSTANCE hInstance = GetModuleHandle(NULL);

		wgpu::SurfaceDescriptorFromWindowsHWND windowDesc;
		windowDesc.hinstance = hInstance;
		windowDesc.hwnd = hWnd;
		windowDesc.sType = wgpu::SType::SurfaceDescriptorFromWindowsHWND;

		wgpu::SurfaceDescriptor surfaceDesc;
		surfaceDesc.nextInChain = &windowDesc;

		return instance.CreateSurface(&surfaceDesc);
	}

	static WGPUSurface GetGLFWSurface(WGPUInstance instance, GLFWwindow* window)
	{
		HWND hWnd = glfwGetWin32Window(window);
		HINSTANCE hInstance = GetModuleHandle(NULL);

		WGPUChainedStruct str{};
		str.sType = WGPUSType_SurfaceDescriptorFromWindowsHWND;

		WGPUSurfaceDescriptorFromWindowsHWND windowDesc{};
		windowDesc.hinstance = hInstance;
		windowDesc.hwnd = hWnd;
		windowDesc.chain = str;

		WGPUSurfaceDescriptor surfaceDesc{};
		surfaceDesc.nextInChain = reinterpret_cast<WGPUChainedStruct*>(&windowDesc);

		WGPUSurface surface = wgpuInstanceCreateSurface(instance, &surfaceDesc);
		wgpuSurfaceReference(surface);
		return surface;
	}

	void GraphicsContext::Init(GLFWwindow* window, uint32_t width, uint32_t height)
	{
		ListAvailableAdapters();

		const WGPUInstanceDescriptor descC{};
		s_InstanceC = wgpuCreateInstance(&descC);

		s_SurfaceC = GetGLFWSurface(s_InstanceC, window);

		WGPURequestAdapterOptions options{};
		options.compatibleSurface = s_SurfaceC;
		options.backendType = WGPUBackendType_D3D12;
		options.powerPreference = WGPUPowerPreference_HighPerformance;

		s_AdapterC = RequestAdapter(s_InstanceC, &options);

		WGPUDeviceDescriptor devDesc{};
		devDesc.label = "Volume Device";
		devDesc.defaultQueue.label = "Volume Queue";

		s_DeviceC = RequestDevice(s_AdapterC, &devDesc);

		wgpuDeviceSetUncapturedErrorCallback(s_DeviceC,[](WGPUErrorType type, char const* message, void* pUserData)
		{
			LOG_ERROR("WebGPU Error: {0}", message);
		}, nullptr);

		wgpuDeviceSetLoggingCallback(s_DeviceC, [](WGPULoggingType type, char const* message, void* userdata)
		{
			LOG_ERROR("WebGPU log: {0}", message);
		}, nullptr);

		s_QueueC = wgpuDeviceGetQueue(s_DeviceC);
		s_DefaultTextureFormatC = WGPUTextureFormat_BGRA8Unorm;

		WGPUSwapChainDescriptor swapChainDesc{};
		swapChainDesc.format = s_DefaultTextureFormatC;
		swapChainDesc.presentMode = WGPUPresentMode_Fifo;
		swapChainDesc.usage = WGPUTextureUsage_RenderAttachment;
		swapChainDesc.height = height;
		swapChainDesc.width = width;

		s_SwapChainC = wgpuDeviceCreateSwapChain(s_DeviceC, s_SurfaceC, &swapChainDesc);
	}

}
