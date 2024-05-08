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
		auto volumeMask = contourFile->Create3DMask(*ctFile, { 4, 0, 0, 0 }, ContourPostProcess::RECONSTRUCT_BRESENHAM |
			ContourPostProcess::PROCESS_NON_DUPLICATES | ContourPostProcess::CLOSING | ContourPostProcess::FILL);

		// Must be called before normalization
		p_OpacityTfCT = std::make_unique<OpacityTF>(4096);
		p_ColorTfCT = std::make_unique<ColorTF>(4096);
		p_OpacityTfCT->CalibrateOnMask(volumeMask, ctFile, { 1, 0, 0, 0 });


		ctFile->NormalizeData();
		p_TexCTData = Texture::CreateFromData(base::GraphicsContext::GetDevice(), base::GraphicsContext::GetQueue(), ctFile->GetVoidPtr(), WGPUTextureDimension_3D, ctFile->GetSize(),
			WGPUTextureFormat_RGBA32Float, WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst, sizeof(glm::vec4), "CT data texture");

		
		m_BGroup.AddTexture(*p_TexCTData, WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroup.AddTexture(*p_OpacityTfCT->GetTexture(), WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroup.AddTexture(*p_ColorTfCT->GetTexture(), WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroup.FinalizeBindGroup(base::GraphicsContext::GetDevice());
		IntializePipeline(pipeline);
	}

	void TFCalibrationApp::OnUpdate(base::Timestep ts)
	{
		p_OpacityTfCT->UpdateTexture();
		p_ColorTfCT->UpdateTexture();
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