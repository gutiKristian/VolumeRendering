#include "include/DicomInfoApp.h"

#include "../file/dicom/DicomReader.h"
#include "../file/FileSystem.h"
#include "../Shader.h"
#include "Base/Log.h"
#include "webgpu/webgpu.h"

namespace med
{
	void DicomInfoApp::OnStart(PipelineBuilder& pipeline)
	{
		// auto contourFile = DicomReader::ReadStructFile("assets\\716^716_716_RTst_2013-04-02_230000_716-1-01_OCM.BladderShell_n1__00000\\");
		// auto ctFile = DicomReader::ReadVolumeFile("assets\\716^716_716_CT_2013-04-02_230000_716-1-01_716-1_n81__00000\\");
		// auto rtDoseFile = DicomReader::ReadVolumeFile("assets\\716^716_716_RTDOSE_2013-04-02_230000_716-1-01\\");
		auto res = DicomReader::GetTag(FileSystem::GetDefaultPath() / "assets\\716^716_716_CT_2013-04-02_230000_716-1-01_716-1_n81__00000\\1.2.246.352.62.1.4617947587420317968.8340828381246895766.dcm", 0x00100010);
		WGPUShaderModule shaderModule = Shader::create_shader_module(base::GraphicsContext::GetDevice(),
			FileSystem::ReadFile(FileSystem::GetDefaultPath() / "shaders" / "DicomInfoApp.wgsl"));
		pipeline.AddShaderModule(shaderModule);
	}

	void DicomInfoApp::OnUpdate(base::Timestep ts)
	{
	}

	void DicomInfoApp::OnRender(WGPURenderPassEncoder pass)
	{
	}

	void DicomInfoApp::OnEnd()
	{
	}

	void DicomInfoApp::OnImGuiRender() const
	{
	}

	void DicomInfoApp::IntializePipeline(PipelineBuilder& pipeline)
	{
	}
}