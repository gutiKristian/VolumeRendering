#pragma once

#include <webgpu/webgpu_cpp.h>

struct GLFWwindow;

namespace base {

	class GraphicsContext
	{
	public:
		GraphicsContext() = delete;

		static void Init(GLFWwindow* window, uint32_t width, uint32_t height);

		static void OnWindowResize(uint32_t width, uint32_t height);

		static wgpu::Device GetDevice() { return s_Device; }
		static wgpu::Surface GetSurface() { return s_Surface; }
		static wgpu::Queue GetQueue() { return s_Queue; }
		static wgpu::SwapChain GetSwapChain() { return s_SwapChain; }
		static wgpu::TextureFormat GetDefaultTextureFormat() { return s_DefaultTextureFormat; }
		static const wgpu::Limits& GetLimits() { return s_Limits; }
	private:
		static wgpu::Adapter RequestAdapter(wgpu::Instance instance, const wgpu::RequestAdapterOptions* options);
		static wgpu::Device RequestDevice(wgpu::Adapter adapter, const wgpu::DeviceDescriptor* descriptor);
	private:
		static inline wgpu::Instance s_Instance;
		static inline wgpu::Adapter s_Adapter;
		static inline wgpu::Device s_Device;
		static inline wgpu::Surface s_Surface;
		static inline wgpu::Queue s_Queue;
		static inline wgpu::SwapChain s_SwapChain;
		static inline wgpu::TextureFormat s_DefaultTextureFormat;
		static inline wgpu::Limits s_Limits;
	};

}
