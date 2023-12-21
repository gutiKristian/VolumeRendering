#include "Sampler.h"

namespace med
{
	Sampler::Sampler(WGPUSampler sampler) : m_Sampler(sampler)
	{
	}

	std::shared_ptr<Sampler> Sampler::CreateSampler(const WGPUDevice& device)
	{
		WGPUSamplerDescriptor samplerDesc{};
		samplerDesc.addressModeU = WGPUAddressMode_ClampToEdge;
		samplerDesc.addressModeV = WGPUAddressMode_ClampToEdge;
		samplerDesc.addressModeW = WGPUAddressMode_ClampToEdge;
		// We want sampler that does not do filtering
		/*samplerDesc.magFilter = WGPUFilterMode_Linear;
		samplerDesc.minFilter = WGPUFilterMode_Linear;
		samplerDesc.mipmapFilter = WGPUFilterMode_Linear;*/
		samplerDesc.lodMinClamp = 0.0f;
		samplerDesc.lodMaxClamp = 1.0f;
		samplerDesc.compare = WGPUCompareFunction_Undefined;
		samplerDesc.maxAnisotropy = 1;
		WGPUSampler sampler = wgpuDeviceCreateSampler(device, &samplerDesc);
		return std::make_shared<Sampler>(sampler);
	}

	WGPUSampler Sampler::GetSampler() const
	{
		return m_Sampler;
	}
}