#include "webgpu/webgpu.h"
#include "RenderPipeline.h"
#include "VertexBuffer.h"
#include "BindGroup.h"
#include "Base/GraphicsContext.h"

#include <vector>
#include <memory>

namespace med
{
	class PipelineBuilder
	{
	public:
		PipelineBuilder();

		/*
		* Add shader to the pipeline. Fragment, vertex modules,
		* blend state and color target are set up here.
		* Default entry points for shaders are: vs_main and fs_main.
		*/
		void AddShaderModule(WGPUShaderModule shader);
		
		void AddBuffer(const VertexBuffer& buffer);

		void AddBindGroup(const BindGroup& bg);

		/*
		* Setting up depth attachment.
		*/
		void DepthTexMagic(uint32_t width, uint32_t height);

		/*
		 * Set the cull face.
		 * Default: Disabled
		 */
		void SetCullFace(WGPUCullMode mode);

		/*
		 * Set whether the front face is CCW or CW triangle.
		 * Default: CCW
		 */
		void SetFrontFace(WGPUFrontFace order);

		/*
		* What format should pipeline output.
		* If this method was not called, format provided by graphics context is used.
		*/
		void SetColorTargetFormat(WGPUTextureFormat format);
	
		/*
		* Creating instance of pipeline. Buffers 
		*/
		std::shared_ptr<RenderPipeline> BuildPipeline();

	private:
		void InitPipelineDefaults();
		void SetDefaultStencilFaceState(WGPUStencilFaceState& stencilFaceState);
		void SetDefaultDepthStencilState(WGPUDepthStencilState& depthStencilState);

	private:
		WGPURenderPipelineDescriptor m_PipelineDesc{};
		WGPUTextureView m_DepthTexView;

		// buffers
		WGPUPipelineLayoutDescriptor m_PipelineLayoutDesc{};
		std::vector<WGPUVertexBufferLayout> m_VertexBufferLayouts;
		std::vector<WGPUBindGroupLayout> m_BindGroupLayouts;
		
		// Need to exist until we create the pipeline
		WGPUDepthStencilState m_DepthStencilStatem{};
		WGPUFragmentState m_FragmentState{};
		WGPUBlendState m_BlendState{};
		WGPUColorTargetState m_ColorTarget{};

		int m_BindGroupsCount = 0;
	};
}