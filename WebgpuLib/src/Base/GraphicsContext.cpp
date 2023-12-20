#include "GraphicsContext.h"

#include "Base/Base.h"
#include "Base/Utils.h"

namespace base {

	wgpu::Adapter GraphicsContext::RequestAdapter(wgpu::Instance instance, const wgpu::RequestAdapterOptions* options)
	{
		struct UserData
		{
			wgpu::Adapter adapter = nullptr;
			bool requestEnded = false;
		} userData;

		auto onAdapterRequestEnded = [](WGPURequestAdapterStatus status, WGPUAdapter adapter, char const* message, void* userdata)
		{
			UserData& userData = *reinterpret_cast<UserData*>(userdata);
			if (status == WGPURequestAdapterStatus_Success)
			{
				userData.adapter = wgpu::Adapter(adapter);
			}
			else
			{
				CRITICAL(message);
				ASSERT(false, "Could not get WebGPU adapter");
			}

			userData.requestEnded = true;
		};

		instance.RequestAdapter(options, onAdapterRequestEnded, &userData);
		while (!userData.requestEnded)
		{
			Utils::Sleep(10);
		}

		ASSERT(userData.requestEnded, "Request has not yet ended");

		wgpu::Adapter adapter = userData.adapter;
		wgpu::SupportedLimits limits;
		adapter.GetLimits(&limits);
		s_Limits = limits.limits;
		return userData.adapter;
	}

	wgpu::Device GraphicsContext::RequestDevice(wgpu::Adapter adapter, const wgpu::DeviceDescriptor* descriptor)
	{
		struct UserData
		{
			WGPUDevice device = nullptr;
			bool requestEnded = false;
		} userData;

		auto onDeviceRequestEnded = [](WGPURequestDeviceStatus status, WGPUDevice device, char const* message, void* pUserData)
			{
				UserData& userData = *reinterpret_cast<UserData*>(pUserData);
				if (status == WGPURequestDeviceStatus_Success)
				{
					userData.device = device;
				}
				else
				{
					CRITICAL(message);
					ASSERT(false, "Could not get WebGPU device");
				}
				userData.requestEnded = true;
			};

		adapter.RequestDevice(descriptor, onDeviceRequestEnded, &userData);
		while (!userData.requestEnded)
		{
			Utils::Sleep(10);
		}

		ASSERT(userData.requestEnded, "Request has not yet ended");
		return userData.device;
	}

	void GraphicsContext::OnWindowResize(uint32_t width, uint32_t height)
	{
		wgpu::SwapChainDescriptor swapChainDesc;
		swapChainDesc.format = s_DefaultTextureFormat;
		swapChainDesc.presentMode = wgpu::PresentMode::Fifo;
		swapChainDesc.usage = wgpu::TextureUsage::RenderAttachment;
		swapChainDesc.width = width;
		swapChainDesc.height = height;

		s_SwapChain = s_Device.CreateSwapChain(s_Surface, &swapChainDesc);
	}

}
