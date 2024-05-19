#include "include/BasicVolLightApp.h"

#include "webgpu/webgpu.h"
#include "Base/GraphicsContext.h"
#include "Base/Base.h"
#include "../Shader.h"
#include "../file/FileSystem.h"
#include "../file/dicom/DicomReader.h"

namespace med
{
	void BasicVolLightApp::OnStart(PipelineBuilder& pipeline)
	{
		LOG_INFO("OnStart Basic volume app with light");

		 //auto ctFile = DicomReader::ReadVolumeFile(FileSystem::GetDefaultPath() / "assets\\HumanHead\\");
		//auto ctFile = DicomReader::ReadVolumeFile(FileSystem::GetDefaultPath() / "assets\\chestCTContrast\\");
		auto ctFile = DicomReader::ReadVolumeFile(FileSystem::GetDefaultPath() / "assets\\716^716_716_CT_2013-04-02_230000_716-1-01_716-1_n81__00000\\");

		ctFile->NormalizeData();
		ctFile->PreComputeGradient();
		ctFile->AverageGradient(5);

		ComputeRecommendedSteppingParams(*ctFile);
		
		p_OpacityTf = std::make_unique<OpacityTF>(4096);
		p_ColorTf = std::make_unique<ColorTF>(4096);

		p_OpacityTf->Load((FileSystem::GetDefaultPath() / "assets\\this").string());
		p_ColorTf->Load((FileSystem::GetDefaultPath() / "assets\\tfcolorLungsContrast4096").string());

		p_OpacityTf->SetDataRange(ctFile->GetMaxNumber());
		p_OpacityTf->ActivateHistogram(*ctFile);

		p_TexData = Texture::CreateFromData(base::GraphicsContext::GetDevice(), base::GraphicsContext::GetQueue(), ctFile->GetVoidPtr(), WGPUTextureDimension_3D, ctFile->GetSize(),
			WGPUTextureFormat_RGBA32Float, WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst, sizeof(glm::vec4), "CT data texture");

		p_ULight = UniformBuffer::CreateFromData(base::GraphicsContext::GetDevice(), base::GraphicsContext::GetQueue(), &m_Light1, sizeof(Light));


		m_BGroup.AddTexture(*p_TexData, WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroup.AddTexture(*p_OpacityTf->GetTexture(), WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroup.AddTexture(*p_ColorTf->GetTexture(), WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroup.AddBuffer(*p_ULight, WGPUShaderStage_Fragment);
		m_BGroup.FinalizeBindGroup(base::GraphicsContext::GetDevice());
		IntializePipeline(pipeline);
	}

	void BasicVolLightApp::OnUpdate(base::Timestep ts)
	{
		p_OpacityTf->UpdateTexture();
		p_ColorTf->UpdateTexture();
	}

	void BasicVolLightApp::OnRender(const WGPURenderPassEncoder pass)
	{
		m_BGroup.Bind(pass);
	}

	void BasicVolLightApp::OnEnd()
	{
		LOG_INFO("OnEnd BasicVolumeMiniApp");
	}

	void BasicVolLightApp::OnImGuiRender() const
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

	void BasicVolLightApp::OnResize(uint32_t width, uint32_t height, PipelineBuilder& pipeline)
	{
		LOG_INFO("Resize BasicVolLightApp");
		IntializePipeline(pipeline);
	}

	void BasicVolLightApp::IntializePipeline(PipelineBuilder& pipeline)
	{
		WGPUShaderModule shaderModule = Shader::create_shader_module(base::GraphicsContext::GetDevice(),
			FileSystem::ReadFile(FileSystem::GetDefaultPath() / "shaders" / "BasicVolLightApp.wgsl"));
		pipeline.AddShaderModule(shaderModule);
		pipeline.AddBindGroup(m_BGroup);
	}

}