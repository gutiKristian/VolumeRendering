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

	void GraphicsContext::Init(GLFWwindow* window, uint32_t width, uint32_t height)
	{
		ListAvailableAdapters();

		wgpu::InstanceDescriptor desc = {};
		s_Instance = wgpu::CreateInstance(&desc);

		s_Surface = GetGLFWSurface(s_Instance, window);

		wgpu::RequestAdapterOptions opts;
		opts.compatibleSurface = s_Surface;
		opts.backendType = wgpu::BackendType::D3D12;
		opts.powerPreference = wgpu::PowerPreference::HighPerformance;

		s_Adapter = RequestAdapter(s_Instance, &opts);

		wgpu::DeviceDescriptor deviceDesc;
		deviceDesc.label = "Volume_Device";
		deviceDesc.defaultQueue.label = "Volume_Queue";

		s_Device = RequestDevice(s_Adapter, &deviceDesc);

		s_Device.SetUncapturedErrorCallback([](WGPUErrorType type, char const* message, void* pUserData)
		{
			ERROR("WebGPU Error: {0}", message);
		}, nullptr);
		s_Device.SetLoggingCallback([](WGPULoggingType type, char const* message, void* userdata)
		{
			ERROR("WebGPU log: {0}", message);
		}, nullptr);

		s_Queue = s_Device.GetQueue();
		s_DefaultTextureFormat = wgpu::TextureFormat::BGRA8Unorm;

		wgpu::SwapChainDescriptor swapChainDesc;
		swapChainDesc.format = s_DefaultTextureFormat;
		swapChainDesc.presentMode = wgpu::PresentMode::Fifo;
		swapChainDesc.usage = wgpu::TextureUsage::RenderAttachment;
		swapChainDesc.width = width;
		swapChainDesc.height = height;

		s_SwapChain = s_Device.CreateSwapChain(s_Surface, &swapChainDesc);
	}

}
