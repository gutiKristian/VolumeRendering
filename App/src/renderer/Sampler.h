#pragma once
#include "webgpu/webgpu.h"
#include <memory>
namespace med
{
	class Sampler
	{
	public:
		explicit Sampler(WGPUSampler sampler);
	public:
		static std::shared_ptr<Sampler> CreateSampler(const WGPUDevice& device);

		WGPUSampler GetSampler() const;
	private:
		WGPUSampler m_Sampler;
	};
}