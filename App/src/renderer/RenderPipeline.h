#pragma once
#include "webgpu/webgpu.h"

namespace med
{
	class RenderPipeline
	{
	public:
		RenderPipeline(WGPURenderPipeline pipeline, WGPUTextureView depthTexView);
	public:
		WGPUTextureView GetDepthTextureView() const;
		void Bind(const WGPURenderPassEncoder& pass) const;
	private:
		WGPURenderPipeline m_PipelineObj;
		WGPUTextureView m_DepthTexView;
	};
}