#include "include/BasicVolumeApp.h"

#include "webgpu/webgpu.h"
#include "Base/GraphicsContext.h"
#include "Base/Base.h"
#include "../Shader.h"
#include "../file/FileSystem.h"
#include "../file/dicom/DicomReader.h"

namespace med
{
	void BasicVolumeApp::OnStart(PipelineBuilder& pipeline)
	{
		LOG_INFO("OnStart BasicVolumeMiniApp");

		auto ctFile = DicomReader::ReadVolumeFile(FileSystem::GetDefaultPath() / "assets\\HumanHead\\");
		ctFile->NormalizeData();

		p_OpacityTf = std::make_unique<OpacityTF>(256);
		p_ColorTf = std::make_unique<ColorTF>(256);

		p_OpacityTf->SetDataRange(ctFile->GetMaxNumber());
		p_OpacityTf->ActivateHistogram(*ctFile);

		p_TexData = Texture::CreateFromData(base::GraphicsContext::GetDevice(), base::GraphicsContext::GetQueue(), ctFile->GetVoidPtr(), WGPUTextureDimension_3D, ctFile->GetSize(),
			WGPUTextureFormat_RGBA32Float, WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst, sizeof(glm::vec4), "CT data texture");
	
		m_BGroup.AddTexture(*p_TexData, WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroup.AddTexture(*p_OpacityTf->GetTexture(), WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroup.AddTexture(*p_ColorTf->GetTexture(), WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroup.FinalizeBindGroup(base::GraphicsContext::GetDevice());
		IntializePipeline(pipeline);
	}

	void BasicVolumeApp::OnUpdate(base::Timestep ts)
	{
		p_OpacityTf->UpdateTexture();
		p_ColorTf->UpdateTexture();
	}

	void BasicVolumeApp::OnRender(const WGPURenderPassEncoder pass)
	{
		m_BGroup.Bind(pass);
	}

	void BasicVolumeApp::OnEnd()
	{
		LOG_INFO("OnEnd BasicVolumeMiniApp");
	}

	void BasicVolumeApp::OnImGuiRender() const
	{
		ImGui::Begin("MiniApp");

		MED_BEGIN_TAB_BAR("BasicVolumeRendering")
		
		MED_BEGIN_TAB_ITEM("Transfer functions")
		p_OpacityTf->Render();
		p_ColorTf->Render();
		MED_END_TAB_ITEM

		MED_END_TAB_BAR
	
		ImGui::End();
	}

	void BasicVolumeApp::OnResize(uint32_t width, uint32_t height, PipelineBuilder& pipeline)
	{
		LOG_INFO("Resize BasicVolumeApp");
		IntializePipeline(pipeline);
	}

	void BasicVolumeApp::IntializePipeline(PipelineBuilder& pipeline)
	{
		WGPUShaderModule shaderModule = Shader::create_shader_module(base::GraphicsContext::GetDevice(),
			FileSystem::ReadFile(FileSystem::GetDefaultPath() / "shaders" / "BasicVolumeApp.wgsl"));
		pipeline.AddShaderModule(shaderModule);
		pipeline.AddBindGroup(m_BGroup);
	}

}