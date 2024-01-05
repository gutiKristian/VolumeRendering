#include "PipelineBuilder.h"
#include <algorithm>
#include <vector>
#include <cassert>

namespace med
{
	PipelineBuilder::PipelineBuilder()
	{
		InitPipelineDefaults();
	}

	void PipelineBuilder::InitPipelineDefaults()
	{
		m_PipelineDesc.nextInChain = nullptr;
		m_PipelineDesc.vertex.bufferCount = 0;
		m_PipelineDesc.vertex.buffers = nullptr;

		// Samples per pixel
		m_PipelineDesc.multisample.count = 1;
		// Default value for the mask, meaning "all bits on"
		m_PipelineDesc.multisample.mask = ~0u;
		// Default value as well (irrelevant for count = 1 anyways)
		m_PipelineDesc.multisample.alphaToCoverageEnabled = false;
	}


	void PipelineBuilder::DepthTexMagic(uint32_t width, uint32_t height)
	{
		//! CREATE TEXTURE OBJECT, SEND THIS TO RENDERPIPELINE
		//! THIS WILL BE UPDATED THERE, THE VIEW MUST BE SEND THERE NEVERTHELESS

		SetDefaultDepthStencilState(m_DepthStencilStatem);
		m_DepthStencilStatem.depthCompare = WGPUCompareFunction_Less;
		m_DepthStencilStatem.depthWriteEnabled = true;
		m_DepthStencilStatem.stencilReadMask = 0;
		m_DepthStencilStatem.stencilWriteMask = 0;

		WGPUTextureFormat depthTexFormat = WGPUTextureFormat_Depth24Plus;
		m_DepthStencilStatem.format = depthTexFormat;
		assert(m_DepthStencilStatem.nextInChain == nullptr);
		m_PipelineDesc.depthStencil = &m_DepthStencilStatem;

		// Create the depth texture
		WGPUTextureDescriptor depthTextureDesc{};
		depthTextureDesc.nextInChain = nullptr;
		depthTextureDesc.dimension = WGPUTextureDimension_2D;
		depthTextureDesc.format = depthTexFormat;
		depthTextureDesc.mipLevelCount = 1;
		depthTextureDesc.sampleCount = 1;
		depthTextureDesc.size = { width, height, 1 };
		depthTextureDesc.usage = WGPUTextureUsage_RenderAttachment;
		depthTextureDesc.viewFormatCount = 1;
		depthTextureDesc.viewFormats = &depthTexFormat;
		
		WGPUTexture depthTexture = wgpuDeviceCreateTexture(base::GraphicsContext::GetDevice(), &depthTextureDesc);

		WGPUTextureViewDescriptor depthTextureViewDesc{};
		depthTextureViewDesc.aspect = WGPUTextureAspect_DepthOnly;
		depthTextureViewDesc.nextInChain = nullptr;
		depthTextureViewDesc.baseArrayLayer = 0;
		depthTextureViewDesc.arrayLayerCount = 1;
		depthTextureViewDesc.baseMipLevel = 0;
		depthTextureViewDesc.mipLevelCount = 1;
		depthTextureViewDesc.dimension = WGPUTextureViewDimension_2D;
		depthTextureViewDesc.format = depthTexFormat;
		m_DepthTexView = wgpuTextureCreateView(depthTexture, &depthTextureViewDesc);
	}

	void PipelineBuilder::SetCullFace(WGPUCullMode mode)
	{
		m_PipelineDesc.primitive.cullMode = mode;
	}

	void PipelineBuilder::SetFrontFace(WGPUFrontFace order)
	{
		m_PipelineDesc.primitive.frontFace = order;
	}

	std::shared_ptr<RenderPipeline> PipelineBuilder::BuildPipeline()
	{
		// Finalize vertex buffers and bindgroups creation
		m_PipelineDesc.vertex.bufferCount = static_cast<uint32_t>(m_VertexBufferLayouts.size());
		m_PipelineDesc.vertex.buffers = m_VertexBufferLayouts.data();
		m_PipelineDesc.layout = wgpuDeviceCreatePipelineLayout(base::GraphicsContext::GetDevice(), &m_PipelineLayoutDesc);

		return std::make_shared<RenderPipeline>(wgpuDeviceCreateRenderPipeline(base::GraphicsContext::GetDevice(), &m_PipelineDesc), m_DepthTexView);
	}


	void PipelineBuilder::SetDefaultStencilFaceState(WGPUStencilFaceState& stencilFaceState)
	{
		stencilFaceState.compare = WGPUCompareFunction_Always;
		stencilFaceState.failOp = WGPUStencilOperation_Keep;
		stencilFaceState.depthFailOp = WGPUStencilOperation_Keep;
		stencilFaceState.passOp = WGPUStencilOperation_Keep;
	}

	void PipelineBuilder::SetDefaultDepthStencilState(WGPUDepthStencilState& depthStencilState)
	{
		depthStencilState.nextInChain = nullptr;
		depthStencilState.format = WGPUTextureFormat_Undefined;
		depthStencilState.depthWriteEnabled = false;
		depthStencilState.depthCompare = WGPUCompareFunction_Always;
		depthStencilState.stencilReadMask = 0xFFFFFFFF;
		depthStencilState.stencilWriteMask = 0xFFFFFFFF;
		depthStencilState.depthBias = 0;
		depthStencilState.depthBiasSlopeScale = 0;
		depthStencilState.depthBiasClamp = 0;
		SetDefaultStencilFaceState(depthStencilState.stencilFront);
		SetDefaultStencilFaceState(depthStencilState.stencilBack);
	}

	void PipelineBuilder::AddShaderModule(WGPUShaderModule shader)
	{

		m_PipelineDesc.vertex.nextInChain = nullptr;
		m_PipelineDesc.vertex.module = shader;
		m_PipelineDesc.vertex.entryPoint = "vs_main";
		m_PipelineDesc.vertex.constantCount = 0;
		m_PipelineDesc.vertex.constants = nullptr;

		m_PipelineDesc.primitive.nextInChain = nullptr;
		m_PipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
		m_PipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
		m_PipelineDesc.primitive.frontFace = WGPUFrontFace_CCW;
		m_PipelineDesc.primitive.cullMode = WGPUCullMode_None;

		m_FragmentState.nextInChain = nullptr;
		m_FragmentState.module = shader;
		m_FragmentState.entryPoint = "fs_main";
		m_FragmentState.constantCount = 0;
		m_FragmentState.constants = nullptr;
		
		m_PipelineDesc.fragment = &m_FragmentState;

		m_BlendState.color.srcFactor = WGPUBlendFactor_SrcAlpha;
		m_BlendState.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
		m_BlendState.color.operation = WGPUBlendOperation_Add;
		m_BlendState.alpha.srcFactor = WGPUBlendFactor_Zero;
		m_BlendState.alpha.dstFactor = WGPUBlendFactor_One;
		m_BlendState.alpha.operation = WGPUBlendOperation_Add;

		m_ColorTarget.nextInChain = nullptr;
		m_ColorTarget.format = base::GraphicsContext::GetDefaultTextureFormat();
		m_ColorTarget.blend = &m_BlendState;
		m_ColorTarget.writeMask = WGPUColorWriteMask_All;
		m_FragmentState.targetCount = 1;
		m_FragmentState.targets = &m_ColorTarget;
	}

	void PipelineBuilder::AddBuffer(const VertexBuffer& buffer)
	{
		m_VertexBufferLayouts.push_back(buffer.GetLayout());
	}

	void PipelineBuilder::AddBindGroup(const BindGroup& bg)
	{
		assert(bg.IsInitialized() && "BindGroup was not initialized!");
		m_BindGroupLayouts.push_back(bg.GetLayout());
		m_PipelineLayoutDesc.nextInChain = nullptr;
		m_PipelineLayoutDesc.bindGroupLayouts = m_BindGroupLayouts.data();
		m_PipelineLayoutDesc.bindGroupLayoutCount = static_cast<uint32_t>(m_BindGroupLayouts.size());
	}

}