#include "include/VolumeMaskApp.h"

#include "webgpu/webgpu.h"
#include "Base/GraphicsContext.h"
#include "Base/Base.h"
#include "../Shader.h"
#include "../file/FileSystem.h"
#include "../file/dicom/DicomReader.h"

namespace med
{
	void VolumeMaskApp::OnStart(PipelineBuilder& pipeline)
	{
		LOG_INFO("OnStart BasicVolumeMiniApp");

		auto contourFile = DicomReader::ReadStructFile("assets\\716^716_716_RTst_2013-04-02_230000_716-1-01_OCM.BladderShell_n1__00000\\");

		contourFile->ListAvailableContours();

		auto rtFile = DicomReader::ReadVolumeFile("assets\\716^716_716_RTDOSE_2013-04-02_230000_716-1-01_Eclipse.Doses.0,.Generated.from.plan.'1.pelvis',.1.pelvis.#,.IN_n1__00000\\");
		auto ctFile = DicomReader::ReadVolumeFile("assets\\716^716_716_CT_2013-04-02_230000_716-1-01_716-1_n81__00000\\");
		ctFile->PreComputeGradient(true);

		auto volumeMask = contourFile->Create3DMask(*ctFile, { 2, 4, 0, 0 }, ContourPostProcess::RECONSTRUCT_BRESENHAM |
			ContourPostProcess::PROCESS_NON_DUPLICATES | ContourPostProcess::CLOSING | ContourPostProcess::FILL);


		p_OpacityTfCT = std::make_unique<OpacityTF>(256);
		p_ColorTfCT = std::make_unique<ColorTF>(256);

		p_OpacityTfRT = std::make_unique<OpacityTF>(4096);
		p_ColorTfRT = std::make_unique<ColorTF>(4096);

		rtFile->NormalizeData();
		ctFile->NormalizeData();

		ComputeRecommendedSteppingParams(*ctFile);

		p_OpacityTfCT->SetDataRange(ctFile->GetDataRange());
		p_OpacityTfCT->ActivateHistogram(*ctFile);

		p_OpacityTfRT->SetDataRange(rtFile->GetDataRange());
		p_OpacityTfRT->ActivateHistogram(*rtFile);


		p_TexData = Texture::CreateFromData(base::GraphicsContext::GetDevice(), base::GraphicsContext::GetQueue(), volumeMask->GetVoidPtr(), WGPUTextureDimension_3D, volumeMask->GetSize(),
			WGPUTextureFormat_RGBA32Float, WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst, sizeof(glm::vec4), "Mask texture");

		p_RTTexData = Texture::CreateFromData(base::GraphicsContext::GetDevice(), base::GraphicsContext::GetQueue(), rtFile->GetVoidPtr(), WGPUTextureDimension_3D, rtFile->GetSize(),
			WGPUTextureFormat_RGBA32Float, WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst, sizeof(glm::vec4), "RT texture");

		p_CTTexData = Texture::CreateFromData(base::GraphicsContext::GetDevice(), base::GraphicsContext::GetQueue(), ctFile->GetVoidPtr(), WGPUTextureDimension_3D, ctFile->GetSize(),
			WGPUTextureFormat_RGBA32Float, WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst, sizeof(glm::vec4), "CT texture");


		m_BGroup.AddTexture(*p_TexData, WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroup.AddTexture(*p_RTTexData, WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroup.AddTexture(*p_CTTexData, WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroup.AddTexture(*p_OpacityTfCT->GetTexture(), WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroup.AddTexture(*p_ColorTfCT->GetTexture(), WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroup.AddTexture(*p_OpacityTfRT->GetTexture(), WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroup.AddTexture(*p_ColorTfRT->GetTexture(), WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroup.FinalizeBindGroup(base::GraphicsContext::GetDevice());
		IntializePipeline(pipeline);
	}

	void VolumeMaskApp::OnUpdate(base::Timestep ts)
	{
		p_OpacityTfCT->UpdateTexture();
		p_OpacityTfRT->UpdateTexture();
		p_ColorTfCT->UpdateTexture();
		p_ColorTfRT->UpdateTexture();
	}

	void VolumeMaskApp::OnRender(const WGPURenderPassEncoder pass)
	{
		m_BGroup.Bind(pass);
	}

	void VolumeMaskApp::OnEnd()
	{
		LOG_INFO("OnEnd BasicVolumeMiniApp");
	}

	void VolumeMaskApp::OnImGuiRender() const
	{
		ImGui::Begin("MiniApp");
		MED_BEGIN_TAB_BAR("BasicVolumeRendering")

		MED_BEGIN_TAB_ITEM("CT Transfer functions")
		p_OpacityTfCT->Render();
		p_ColorTfCT->Render();
		MED_END_TAB_ITEM
	
		MED_BEGIN_TAB_ITEM("RT Transfer functions")
		p_OpacityTfRT->Render();
		p_ColorTfRT->Render();
		MED_END_TAB_ITEM
		
		MED_END_TAB_BAR
		ImGui::End();
	}

	void VolumeMaskApp::OnResize(uint32_t width, uint32_t height, PipelineBuilder& pipeline)
	{
		LOG_INFO("Resize VolumeMaskApp");
		IntializePipeline(pipeline);
	}

	void VolumeMaskApp::IntializePipeline(PipelineBuilder& pipeline)
	{
		WGPUShaderModule shaderModule = Shader::create_shader_module(base::GraphicsContext::GetDevice(),
			FileSystem::ReadFile(FileSystem::GetDefaultPath() / "shaders" / "VolumeMaskApp.wgsl"));
		pipeline.AddShaderModule(shaderModule);
		pipeline.AddBindGroup(m_BGroup);
	}

}