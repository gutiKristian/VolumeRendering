#include "ImGuiLayer.h"
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_wgpu.h>

namespace med
{
	void ImGuiLayer::InitializeContext(GLFWwindow* windowHandle)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.IniFilename = nullptr;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForOther(windowHandle, true);
		ImGui_ImplWGPU_Init(base::GraphicsContext::GetDevice(), 3, base::GraphicsContext::GetDefaultTextureFormat());
	}

	void ImGuiLayer::Begin()
	{
		ImGui_ImplWGPU_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiLayer::End(WGPUCommandEncoder encoder)
	{
		WGPURenderPassColorAttachment colorAttachment = {
		.view = wgpuSwapChainGetCurrentTextureView(base::GraphicsContext::GetSwapChain()),
		.loadOp = WGPULoadOp_Load,
		.storeOp = WGPUStoreOp_Store,
		.clearValue = {0.0, 0.0, 0.0, 1.0}
		};

		WGPURenderPassDescriptor renderPassDescriptor = {
			.label = "ImGuiRenderPass",
			.colorAttachmentCount = 1,
			.colorAttachments = &colorAttachment,
			.depthStencilAttachment = nullptr,
		};

		ImGui::Render();
		const WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDescriptor);
		ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), pass);
		wgpuRenderPassEncoderEnd(pass);
	}

	void ImGuiLayer::Destroy()
	{
		// ifndef web
		ImGui_ImplWGPU_Shutdown();
		ImGui_ImplGlfw_Shutdown();
	}
}
