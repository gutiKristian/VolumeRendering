#include "GraphicsContext.h"

#include "Base/Base.h"
#include "Base/Utils.h"
#include "dawn/native/DawnNative.h"

#include <sstream>

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

	std::string GraphicsContext::ResolveBackendType(wgpu::BackendType type)
	{
		switch (type)
		{
		case wgpu::BackendType::Undefined:
			return "Undefined";
		case wgpu::BackendType::Null:
			return "Null";
		case wgpu::BackendType::WebGPU:
			return "WebGPU";
		case wgpu::BackendType::D3D11:
			return "D3D11";
		case wgpu::BackendType::D3D12:
			return "D3D12";
		case wgpu::BackendType::Metal:
			return "Metal";
		case wgpu::BackendType::Vulkan:
			return "Vulkan";
		case wgpu::BackendType::OpenGL:
			return "OpenGL";
		case wgpu::BackendType::OpenGLES:
			return "OpenGLES";
		}
		return "Please update backend type definitions in Resolve method";
	}

	std::string GraphicsContext::ResolveAdapterType(wgpu::AdapterType type)
	{
		switch (type)
		{
		case wgpu::AdapterType::DiscreteGPU:
			return "Discrete GPU";
		case wgpu::AdapterType::IntegratedGPU:
			return "Integrated GPU";
		case wgpu::AdapterType::CPU:
			return "CPU";
		case wgpu::AdapterType::Unknown:
			return "Unknown";
		}
		return "Please update adapter type definitions in Resolve method";
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

	/*
	 * This method has only informative character, so user knows what adapters
	 * are available. The method uses dawn::native version, these versions might be less stable
	 * or use some internal chromium api.
	 */
	void GraphicsContext::ListAvailableAdapters()
	{
		const auto instance = dawn::native::Instance();
		const std::vector<dawn::native::Adapter> adapters = instance.EnumerateAdapters();

		std::stringstream log;
		log << "Available adapters:\n";

		for (const auto& adapter: adapters)
		{
			wgpu::AdapterProperties prop;
			adapter.GetProperties(&prop);
			std::vector<const char*> features = adapter.GetSupportedFeatures();

			log << "\n";
			log << "Adapter type: " << ResolveAdapterType(prop.adapterType) << "\n";
			log << "Name: " << prop.name << "\n" << "Architecture: " << prop.architecture << "\n";
			log << "Vendor: " << prop.vendorName << "\n" << "Driver description: " << prop.driverDescription << "\n";
			log << "Backend: " << ResolveBackendType(prop.backendType) << "\n";
			log << "List of supported features:\n";

			int i = 1;
			for (const auto* ptr : features)
			{
				log << ptr << ((i++ % 3 == 0) ? "\n" : "\t\t");
			}
			log << "\n";
		}
		std::string result = log.str();
		INFO(result.c_str());
	}
}
