#include "RenderPipeline.h"

namespace med
{
	RenderPipeline::RenderPipeline(WGPURenderPipeline pipeline, WGPUTextureView depthTexView) : m_PipelineObj(pipeline), m_DepthTexView(depthTexView)
	{
	}

	WGPUTextureView RenderPipeline::GetDepthTextureView() const
	{
		return m_DepthTexView;
	}

	void RenderPipeline::Bind(const WGPURenderPassEncoder& pass) const
	{
		wgpuRenderPassEncoderSetPipeline(pass, m_PipelineObj);
	}
}