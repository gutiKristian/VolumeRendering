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
		 		
		DemoBasic();
		//DemoCTReuse();
		//DemoMRIReuse();
		
			
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

	void BasicVolumeApp::DemoBasic()
	{
		//auto ctFile = DicomReader::ReadVolumeFile(FileSystem::GetDefaultPath() / "assets\\HumanHead\\"); 
		//auto ctFile = DicomReader::ReadVolumeFile(FileSystem::GetDefaultPath() / "assets\\chestCTContrast\\");

		//auto ctFile = DicomReader::ReadVolumeFile(FileSystem::GetDefaultPath() / "assets\\CovidLungs1\\");
		//auto ctFile = DicomReader::ReadVolumeFile(FileSystem::GetDefaultPath() / "assets\\chestCTContrast\\");
		auto ctFile = DicomReader::ReadVolumeFile(FileSystem::GetDefaultPath() / "assets\\799_799_CT_2013-07-30_070000_GU_Helical.AutomA_n191__00000\\"); //799_799_CT_2013-07-30_070000_GU_Helical.AutomA_n191__00000 716^716_716_CT_2013-04-02_230000_716-1-01_716-1_n81__00000 
	
		ctFile->NormalizeData();
		ComputeRecommendedSteppingParams(*ctFile);

		p_OpacityTf = std::make_unique<OpacityTF>(256);
		p_OpacityTf->SetDataRange(ctFile->GetDataRange());
		p_OpacityTf->ActivateHistogram(*ctFile);

		p_ColorTf = std::make_unique<ColorTF>(256);
		p_TexData = Texture::CreateFromData(base::GraphicsContext::GetDevice(), base::GraphicsContext::GetQueue(), ctFile->GetVoidPtr(), WGPUTextureDimension_3D, ctFile->GetSize(),
			WGPUTextureFormat_RGBA32Float, WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst, sizeof(glm::vec4), "CT data texture");
	}

	void BasicVolumeApp::DemoCTReuse()
	{
		auto ctFile = DicomReader::ReadVolumeFile(FileSystem::GetDefaultPath() / "assets\\HumanHead\\");
		ctFile->NormalizeData();

		p_OpacityTf = std::make_unique<OpacityTF>(4096);
		p_OpacityTf->SetDataRange(ctFile->GetDataRange());
		
		// Bones2500 defined on 716 dataset
		p_OpacityTf->Load((FileSystem::GetDefaultPath() / "assets\\bones2500").string(), TFLoadOption::RESCALE_TO_NEW_RANGE);
		
		p_OpacityTf->ActivateHistogram(*ctFile);
		p_ColorTf = std::make_unique<ColorTF>(4096);
		p_TexData = Texture::CreateFromData(base::GraphicsContext::GetDevice(), base::GraphicsContext::GetQueue(), ctFile->GetVoidPtr(), WGPUTextureDimension_3D, ctFile->GetSize(),
			WGPUTextureFormat_RGBA32Float, WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst, sizeof(glm::vec4), "CT data texture");
	}

	void BasicVolumeApp::DemoMRIReuse()
	{
		auto mriFile = DicomReader::ReadVolumeFile(FileSystem::GetDefaultPath() / "assets\\pelvis1MRI\\");

		mriFile->NormalizeData();

		p_OpacityTf = std::make_unique<OpacityTF>(256);
		p_OpacityTf->SetDataRange(mriFile->GetDataRange());

		//p_OpacityTf->Load((FileSystem::GetDefaultPath() / "assets\\bones2500").string(), TFLoadOption::RESCALE_TO_NEW_RANGE);

		p_OpacityTf->ActivateHistogram(*mriFile);
		p_ColorTf = std::make_unique<ColorTF>(256);
		p_TexData = Texture::CreateFromData(base::GraphicsContext::GetDevice(), base::GraphicsContext::GetQueue(), mriFile->GetVoidPtr(), WGPUTextureDimension_3D, mriFile->GetSize(),
			WGPUTextureFormat_RGBA32Float, WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst, sizeof(glm::vec4), "CT data texture");
	}

}