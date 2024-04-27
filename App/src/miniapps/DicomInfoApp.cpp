#include "include/DicomInfoApp.h"

#include "../file/dicom/DicomReader.h"
#include "../file/FileSystem.h"
#include "../Shader.h"
#include "Base/Log.h"
#include "webgpu/webgpu.h"
#include "imgui/imgui.h"

namespace med
{
	void DicomInfoApp::OnStart(PipelineBuilder& pipeline)
	{
		auto ctFile = DicomReader::ReadVolumeFile(FileSystem::GetDefaultPath() / "assets\\716^716_716_CT_2013-04-02_230000_716-1-01_716-1_n81__00000\\");
		auto contourFile = DicomReader::ReadStructFile(FileSystem::GetDefaultPath() / "assets\\716^716_716_RTst_2013-04-02_230000_716-1-01_OCM.BladderShell_n1__00000\\");
		auto mask = contourFile->Create3DMask(*ctFile, { 2, 1, 3, 4 }, ContourPostProcess::RECONSTRUCT_BRESENHAM | ContourPostProcess::CLOSING | ContourPostProcess::FILL);
		// auto rtDoseFile = DicomReader::ReadVolumeFile("assets\\716^716_716_RTDOSE_2013-04-02_230000_716-1-01\\");
		IntializePipeline(pipeline);
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
		ImGui::Begin("Dicom Info");
		ImGui::ShowDemoWindow();
		ImGui::End();
	}

	void DicomInfoApp::IntializePipeline(PipelineBuilder& pipeline)
	{
		WGPUShaderModule shaderModule = Shader::create_shader_module(base::GraphicsContext::GetDevice(),
			FileSystem::ReadFile(FileSystem::GetDefaultPath() / "shaders" / "DicomInfoApp.wgsl"));
		pipeline.AddShaderModule(shaderModule);
	}
}