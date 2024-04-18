#include "include/ThreeFilesApp.h"
#include "../Shader.h"
#include "../file/FileReader.h"
#include "../file/dicom/DicomReader.h"
#include "Base/Log.h"

namespace med
{
	void ThreeFilesApp::OnStart(PipelineBuilder& pipeline)
	{
		LOG_WARN("OnStsart ThreeFilesApp");
		auto contourFile = DicomReader::ReadStructFile("assets\\716^716_716_RTst_2013-04-02_230000_716-1-01_OCM.BladderShell_n1__00000\\");
		auto ctFile = DicomReader::ReadVolumeFile("assets\\716^716_716_CT_2013-04-02_230000_716-1-01_716-1_n81__00000\\");
		auto rtDoseFile = DicomReader::ReadVolumeFile("assets\\716^716_716_RTDOSE_2013-04-02_230000_716-1-01\\");
		auto volumeMask = contourFile->Create3DMask(*ctFile, { 2, 0, 0, 0 }, ContourPostProcess::RECONSTRUCT_BRESENHAM | 
			ContourPostProcess::PROCESS_NON_DUPLICATES | ContourPostProcess::CLOSING);

		p_OpacityTfCT = std::make_unique<OpacityTF>(256);
		p_OpacityTfRT = std::make_unique<OpacityTF>(256);
		p_ColorTfCT = std::make_unique<ColorTF>(256);
		p_ColorTfRT = std::make_unique<ColorTF>(256);

		p_TexCTData = Texture::CreateFromData(base::GraphicsContext::GetDevice(), base::GraphicsContext::GetQueue(), ctFile->GetVoidPtr(), WGPUTextureDimension_3D, ctFile->GetSize(),
			WGPUTextureFormat_RGBA32Float, WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst, sizeof(glm::vec4), "CT data texture");

		p_TexRTData = Texture::CreateFromData(base::GraphicsContext::GetDevice(), base::GraphicsContext::GetQueue(), rtDoseFile->GetVoidPtr(), WGPUTextureDimension_3D, rtDoseFile->GetSize(),
			WGPUTextureFormat_RGBA32Float, WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst, sizeof(glm::vec4), "RTDose data texture");

		p_TexMaskData = Texture::CreateFromData(base::GraphicsContext::GetDevice(), base::GraphicsContext::GetQueue(), volumeMask->GetVoidPtr(), WGPUTextureDimension_3D, volumeMask->GetSize(),
			WGPUTextureFormat_RGBA32Float, WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst, sizeof(glm::vec4), "Contour 3D mask");

		p_ULight = UniformBuffer::CreateFromData(base::GraphicsContext::GetDevice(), base::GraphicsContext::GetQueue(), &m_Light1, sizeof(Light));

		m_BGroup.AddTexture(*p_TexCTData, WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroup.AddTexture(*p_TexRTData, WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroup.AddTexture(*p_TexMaskData, WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroup.AddTexture(*p_OpacityTfCT->GetTexture(), WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroup.AddTexture(*p_ColorTfCT->GetTexture(), WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroup.AddTexture(*p_OpacityTfRT->GetTexture(), WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroup.AddTexture(*p_ColorTfRT->GetTexture(), WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroup.AddBuffer(*p_ULight, WGPUShaderStage_Fragment);
		m_BGroup.FinalizeBindGroup(base::GraphicsContext::GetDevice());
		IntializePipeline(pipeline);
	}

	void ThreeFilesApp::OnUpdate(base::Timestep ts)
	{
		p_OpacityTfCT->UpdateTexture();
		p_OpacityTfRT->UpdateTexture();
		p_ColorTfCT->UpdateTexture();
		p_ColorTfRT->UpdateTexture();
	}

	void ThreeFilesApp::OnRender(const WGPURenderPassEncoder pass)
	{
		m_BGroup.Bind(pass);
	}

	void ThreeFilesApp::OnEnd()
	{
	}

	void ThreeFilesApp::OnImGuiRender() const
	{

		MED_BEGIN_TAB_BAR("BasicVolumeRendering")

		MED_BEGIN_TAB_ITEM("CT Transfer Function")
		p_OpacityTfCT->Render();
		p_ColorTfCT->Render();
		MED_END_TAB_ITEM
		
		MED_BEGIN_TAB_ITEM("RT Transfer Function")
		p_OpacityTfRT->Render();
		p_ColorTfRT->Render();
		MED_END_TAB_ITEM

		MED_END_TAB_BAR
	}

	void ThreeFilesApp::OnResize(uint32_t width, uint32_t height, PipelineBuilder& pipeline)
	{
		LOG_INFO("OnResize ThreeFiles");
		IntializePipeline(pipeline);
	}

	void ThreeFilesApp::IntializePipeline(PipelineBuilder& pipeline)
	{
		WGPUShaderModule shaderModule = Shader::create_shader_module(base::GraphicsContext::GetDevice(),
			FileReader::ReadFile(FileReader::GetDefaultPath() / "shaders" / "ThreeFilesApp.wgsl"));
		pipeline.AddShaderModule(shaderModule);
		pipeline.AddBindGroup(m_BGroup);
	}


}