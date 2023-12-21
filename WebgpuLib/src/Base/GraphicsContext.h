#pragma once

#include <webgpu/webgpu_cpp.h>
#include <string>

struct GLFWwindow;

namespace base {

	class GraphicsContext
	{
	public:
		GraphicsContext() = delete;

		static void Init(GLFWwindow* window, uint32_t width, uint32_t height);

		static void OnWindowResize(uint32_t width, uint32_t height);

		static void ListAvailableAdapters();

		static WGPUDevice GetDevice() { return s_DeviceC; }
		static WGPUSurface GetSurface() { return s_SurfaceC; }
		static WGPUQueue GetQueue() { return s_QueueC; }
		static WGPUSwapChain GetSwapChain() { return s_SwapChainC; }
		static WGPUTextureFormat GetDefaultTextureFormat() { return s_DefaultTextureFormatC; }
		static const WGPULimits& GetLimits() { return s_LimitsC; }
	private:
		static WGPUAdapter RequestAdapter(WGPUInstance instance, const WGPURequestAdapterOptions* options);
		static WGPUDevice RequestDevice(WGPUAdapter adapter, const WGPUDeviceDescriptor* descriptor);
		static std::string ResolveBackendType(wgpu::BackendType type);
		static std::string ResolveAdapterType(wgpu::AdapterType type);
	private:
		static inline WGPUInstance s_InstanceC;
		static inline WGPUAdapter s_AdapterC;
		static inline WGPUDevice s_DeviceC;
		static inline WGPUSurface s_SurfaceC;
		static inline WGPUQueue s_QueueC;
		static inline WGPUSwapChain s_SwapChainC;
		static inline WGPUTextureFormat s_DefaultTextureFormatC;
		static inline WGPULimits s_LimitsC;
	};

}
