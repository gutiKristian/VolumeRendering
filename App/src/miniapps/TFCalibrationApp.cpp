#include "include/TFCalibrationApp.h"
#include "../Shader.h"
#include "../file/FileSystem.h"
#include "../file/dicom/DicomReader.h"
#include "../tf/TfUtils.h"
#include "Base/Log.h"

namespace med
{
	void TFCalibrationApp::OnStart(PipelineBuilder& pipeline)
	{
		LOG_WARN("OnStsart TFCalibrationApp");
		auto contourFile = DicomReader::ReadStructFile("assets\\716^716_716_RTst_2013-04-02_230000_716-1-01_OCM.BladderShell_n1__00000\\");
		auto ctFile = DicomReader::ReadVolumeFile("assets\\716^716_716_CT_2013-04-02_230000_716-1-01_716-1_n81__00000\\");
		auto rtDoseFile = DicomReader::ReadVolumeFile("assets\\716^716_716_RTDOSE_2013-04-02_230000_716-1-01_Eclipse.Doses.0,.Generated.from.plan.'1.pelvis',.1.pelvis.#,.IN_n1__00000\\");
		auto volumeMask = contourFile->Create3DMask(*ctFile, { 2, 1, 0, 0 }, ContourPostProcess::RECONSTRUCT_BRESENHAM |
			ContourPostProcess::PROCESS_NON_DUPLICATES | ContourPostProcess::CLOSING | ContourPostProcess::FILL);

		auto tf1 = TfUtils::CalibrateOpacityTF(volumeMask, ctFile, { 1, 0, 0, 0 }, 256);

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


		m_BGroup.AddTexture(*p_TexCTData, WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroup.AddTexture(*p_TexRTData, WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroup.AddTexture(*p_TexMaskData, WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroup.AddTexture(*p_OpacityTfCT->GetTexture(), WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroup.AddTexture(*p_ColorTfCT->GetTexture(), WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroup.AddTexture(*p_OpacityTfRT->GetTexture(), WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroup.AddTexture(*p_ColorTfRT->GetTexture(), WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroup.FinalizeBindGroup(base::GraphicsContext::GetDevice());
		IntializePipeline(pipeline);
	}

	void TFCalibrationApp::OnUpdate(base::Timestep ts)
	{
		p_OpacityTfCT->UpdateTexture();
		p_OpacityTfRT->UpdateTexture();
		p_ColorTfCT->UpdateTexture();
		p_ColorTfRT->UpdateTexture();
	}

	void TFCalibrationApp::OnRender(const WGPURenderPassEncoder pass)
	{
		m_BGroup.Bind(pass);
	}

	void TFCalibrationApp::OnEnd()
	{
	}

	void TFCalibrationApp::OnImGuiRender() const
	{
		ImGui::Begin("MiniApp");

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

			ImGui::End();
	}

	void TFCalibrationApp::OnResize(uint32_t width, uint32_t height, PipelineBuilder& pipeline)
	{
		LOG_INFO("OnResize ThreeFiles");
		IntializePipeline(pipeline);
	}

	void TFCalibrationApp::IntializePipeline(PipelineBuilder& pipeline)
	{
		WGPUShaderModule shaderModule = Shader::create_shader_module(base::GraphicsContext::GetDevice(),
			FileSystem::ReadFile(FileSystem::GetDefaultPath() / "shaders" / "TFCalibrationApp.wgsl"));
		pipeline.AddShaderModule(shaderModule);
		pipeline.AddBindGroup(m_BGroup);
	}

}