#include"Application.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <webgpu/webgpu_cpp.h>
#include <numeric>

#include "Base/Base.h"
#include "Base/Filesystem.h"
#include "Base/Timer.h"
#include "Base/GraphicsContext.h"

#include "file/FileSystem.h"
#include "file/dicom/DicomReader.h"
#include "file/dicom/VolumeFileDcm.h"

#include "Shader.h"
#include "ImGuiLayer.h"
#include "implot.h"
#include "implot_internal.h"

#include "tf/LinearInterpolation.h"

#include "miniapps/include/BasicVolumeApp.h"
#include "miniapps/include/TFCalibrationApp.h"

#if defined(PLATFORM_WEB)
	#include <emscripten.h>
	#include <emscripten/html5.h>
	#include <emscripten/html5_webgpu.h>
#endif

namespace med {

	Application::Application()
	{
		base::Log::Init();
		base::Filesystem::Init();

		base::WindowProps props{ .width = m_Width, .height = m_Height, .title = "Volume Rendering" };
		m_Window = new base::Window(props);

		m_FpsWindow.resize(ROLLING_WINDOW_SIZE, 0.0f);
		m_FrameTimeWindow.resize(ROLLING_WINDOW_SIZE, 0.0f);

		OnStart();
	}

	Application::~Application()
	{
		delete m_Window;
	}

	void Application::OnStart()
	{
		LOG_INFO("On start");
		ImGuiLayer::InitializeContext(m_Window->GetWindowHandle());
		InitializeSamplers();
		InitializeUniforms();
		InitializeTextures();
		InitializeVertexBuffers();
		InitializeIndexBuffers();
		InitializeBindGroups();
		InitializeRenderPipelines();
		p_App = std::make_unique<BasicVolumeApp>();
		p_App->OnStart(m_Builder);
		p_RenderPipeline = m_Builder.BuildPipeline();
	}

	void Application::OnUpdate(base::Timestep ts)
	{
		const auto& queue = base::GraphicsContext::GetQueue();
		constexpr size_t sizeOfInt = sizeof(int);
		constexpr size_t sizeOfFloat = sizeof(float);

		p_UCamera->UpdateBuffer(queue, 64, &m_Camera.GetViewMatrix(), sizeOfFloat * 16);
		p_UCamera->UpdateBuffer(queue, 128, &m_Camera.GetProjectionMatrix(), sizeOfFloat * 16);
		p_UCamera->UpdateBuffer(queue, 192, &m_Camera.GetInverseViewMatrix(), sizeOfFloat * 16);
		p_UCamera->UpdateBuffer(queue, 256, &m_Camera.GetInverseProjectionMatrix(), sizeOfFloat * 16);

		p_UCameraPos->UpdateBuffer(queue, 0, glm::value_ptr(m_Camera.GetPosition()), sizeof(glm::vec3));

		p_UFragmentMode->UpdateBuffer(queue, 0, &m_FragmentMode, sizeOfInt);
		p_UStepsCount->UpdateBuffer(queue, 0, &m_StepsCount, sizeOfInt);
		p_UStepSize->UpdateBuffer(queue, 0, &m_StepSize, sizeOfFloat);

		p_UClipX->UpdateBuffer(queue, 0, glm::value_ptr(m_ClipsX), sizeof(glm::vec2));
		p_UClipY->UpdateBuffer(queue, 0, glm::value_ptr(m_ClipsY), sizeof(glm::vec2));
		p_UClipZ->UpdateBuffer(queue, 0, glm::value_ptr(m_ClipsZ), sizeof(glm::vec2));
		p_UToggles->UpdateBuffer(queue, 0, &m_Toggles, sizeof(glm::ivec4));

		p_App->OnUpdate(ts);
	}

	void Application::OnRender()
	{
		const WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(base::GraphicsContext::GetDevice(), nullptr);

		// ---------- Background ----------
		{
			WGPURenderPassColorAttachment colorAttachmentsBackground = {
				.view = wgpuSwapChainGetCurrentTextureView(base::GraphicsContext::GetSwapChain()),
				.loadOp = WGPULoadOp_Clear,
				.storeOp = WGPUStoreOp_Store,
				.clearValue = {0.0, 0.0, 0.0, 1.0}
			};

			WGPURenderPassDescriptor renderPassDescBackground = {
				.label = "Background Renderpass",
				.colorAttachmentCount = 1,
				.colorAttachments = &colorAttachmentsBackground,
				.depthStencilAttachment = nullptr
			};

			WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDescBackground);
			p_RenderPipelineBackground->Bind(pass);

			wgpuRenderPassEncoderDraw(pass, 6, 1, 0, 0);
			wgpuRenderPassEncoderEnd(pass);
			wgpuRenderPassEncoderRelease(pass);
		}
		
		// ---------- Ray end ----------
		{
			WGPURenderPassColorAttachment colorAttachmentsRay = {
				.view = p_TexEndPos->GetTextureView(),
				.loadOp = WGPULoadOp_Clear,
				.storeOp = WGPUStoreOp_Store,
				.clearValue = {0.0, 0.0, 0.0, 1.0}
			};

			WGPURenderPassDescriptor renderPassDescRay = {
				.label = "Ray end render pass",
				.colorAttachmentCount = 1,
				.colorAttachments = &colorAttachmentsRay,
				.depthStencilAttachment = nullptr
			};

			const WGPURenderPassEncoder passRayEnd = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDescRay);

			p_RenderPipelineEnd->Bind(passRayEnd);
			p_VBCube->Bind(passRayEnd);
			m_BGroupProxy.Bind(passRayEnd);
			wgpuRenderPassEncoderSetIndexBuffer(passRayEnd, p_IBCube->GetBufferPtr(), WGPUIndexFormat_Uint16, 0, p_IBCube->GetSize());
			wgpuRenderPassEncoderDrawIndexed(passRayEnd, p_IBCube->GetCount(), 1, 0, 0, 0);
			wgpuRenderPassEncoderEnd(passRayEnd);
			BindGroup::ResetBindSlotsIndices();

			wgpuRenderPassEncoderRelease(passRayEnd);
		}
		
		// -------------------

		WGPURenderPassDepthStencilAttachment depthStencilAttachment{};
		depthStencilAttachment.view = p_RenderPipeline->GetDepthTextureView();
		depthStencilAttachment.depthClearValue = 1.0f;
		depthStencilAttachment.depthLoadOp = WGPULoadOp_Clear;
		depthStencilAttachment.depthStoreOp = WGPUStoreOp_Store;
		// we could turn off writing to the depth buffer globally here
		depthStencilAttachment.depthReadOnly = false;

		// Stencil setup, mandatory but unused
		constexpr float NaNf = std::numeric_limits<float>::quiet_NaN();
		depthStencilAttachment.stencilClearValue = static_cast<uint32_t>(NaNf);
		depthStencilAttachment.stencilLoadOp = WGPULoadOp_Undefined;
		depthStencilAttachment.stencilStoreOp = WGPUStoreOp_Undefined;
		depthStencilAttachment.stencilReadOnly = true;


		WGPURenderPassColorAttachment colorAttachments = {
			.view = wgpuSwapChainGetCurrentTextureView(base::GraphicsContext::GetSwapChain()),
			.loadOp = WGPULoadOp_Load,
			.storeOp = WGPUStoreOp_Store,
			.clearValue = {0.0, 0.0, 0.0, 1.0}
		};

		WGPURenderPassDescriptor renderPassDesc = {
			.label = "Volume renderpass",
			.colorAttachmentCount = 1,
			.colorAttachments = &colorAttachments,
			.depthStencilAttachment = &depthStencilAttachment,
		};

		WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);

		wgpuRenderPassEncoderSetIndexBuffer(pass, p_IBCube->GetBufferPtr(), WGPUIndexFormat_Uint16, 0, p_IBCube->GetSize());

		p_RenderPipeline->Bind(pass);
		p_VBCube->Bind(pass);
		m_BGroupDefaultApp.Bind(pass);
		p_App->OnRender(pass);
		wgpuRenderPassEncoderDrawIndexed(pass, p_IBCube->GetCount(), 1, 0, 0, 0);
		wgpuRenderPassEncoderEnd(pass);
		BindGroup::ResetBindSlotsIndices();

		// ImGui
		ImGuiLayer::Begin();
		OnImGuiRender();
		ImGuiLayer::End(encoder);

		// Submit
		const WGPUCommandBuffer cmdBuffer = wgpuCommandEncoderFinish(encoder, nullptr);
		wgpuQueueSubmit(base::GraphicsContext::GetQueue(), 1, &cmdBuffer);

#ifndef defined(PLATFORM_WEB)
		wgpuSwapChainPresent(base::GraphicsContext::GetSwapChain());
#endif

		wgpuRenderPassEncoderRelease(pass);             // release pass
		wgpuCommandEncoderRelease(encoder);             // release encoder
		wgpuCommandBufferRelease(cmdBuffer);           // release commands
		wgpuTextureViewRelease(colorAttachments.view); // release textureView
	}

	void Application::OnImGuiRender()
	{
		ImGui::Begin("Main settings");
		
		ImGui::SeparatorText("General");
		{
			ImGui::ListBox("##", &m_FragmentMode, m_FragModes, 5);
			ImGui::SliderInt("Number of steps", &m_StepsCount, 0, 1500);
			ImGui::SliderFloat("Step size", &m_StepSize, 0.001f, 1.0f);
		}

		ImGui::SeparatorText("Clipping");
		{
			ImGui::SliderFloat2("X", glm::value_ptr(m_ClipsX), 0.0f, 0.5f);
			ImGui::SliderFloat2("Y", glm::value_ptr(m_ClipsY), 0.0f, 0.5f);
			ImGui::SliderFloat2("Z", glm::value_ptr(m_ClipsZ), 0.0f, 0.5f);
		}

		ImGui::SeparatorText("Toggles");
		{
			if (ImGui::Checkbox("Variable step size", &m_BToggles[0]))
			{
				m_Toggles[0] = static_cast<int>(m_BToggles[0]);
			}
			ImGui::SetItemTooltip("Step size is calculated to conform the steps count along the ray");

			if (ImGui::Checkbox("Use jittering", &m_BToggles[1]))
			{
				m_Toggles[1] = static_cast<int>(m_BToggles[1]);
			}
			ImGui::SetItemTooltip("Jittering offsets the starting position of the ray, to reduce artifacts.");
		}

		ImGui::End();

		ImGui::Begin("Performance");
		ImGui::SeparatorText("Current");
		{
			ImGui::Text("Frame time:");
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), std::to_string(m_FrameTime).c_str());
			ImGui::Text("FPS:");
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), std::to_string(m_Fps).c_str());
		}
		ImGui::SeparatorText("Average");
		{
			ImGui::Text("Frame time:");
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), std::to_string(m_AverageFrametime).c_str());
			ImGui::Text("FPS:");
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), std::to_string(m_AverageFps).c_str());
		}

		ImGui::End();

		p_App->OnImGuiRender();
	}

	void Application::OnResize(uint32_t width, uint32_t height)
	{
		std::stringstream ss;
		ss << "Application resize\n" <<
			"Updating dimensions\n" <<
			"Width: " << std::to_string(width) <<
			"\nHeight: " << std::to_string(height);
		LOG_INFO(ss.str().c_str());

		m_Width = width;
		m_Height = height;

		// Reinitialize swapchain
		base::GraphicsContext::OnWindowResize(width, height);

		p_TexEndPos = Texture::CreateRenderAttachment(width, height, WGPUTextureUsage_TextureBinding, "Back Faces Texture");

		// Reinit bindgroups and pipelines
		InitializeBindGroups();
		InitializeRenderPipelines();
		
		p_App->OnResize(m_Width, m_Height, m_Builder);

		p_RenderPipeline = m_Builder.BuildPipeline();
	}

	void Application::OnEnd()
	{
		LOG_INFO("On end");
		ImGuiLayer::Destroy();
		p_App->OnEnd();
	}

	void Application::Run()
	{
		LOG_INFO("Run Application");
		m_Running = true;
#if defined(PLATFORM_WEB)
		emscripten_request_animation_frame_loop(Application::EMSRedraw, (void*)this);
#else
		base::Timer timer;

		double prevTime = 0.0;
		double currentTime = 0.0;
		double timeDiff = 0.0;
		unsigned int counter = 0;
		constexpr double updateRat = 1.0 / 30.0;

		int windowIndex = 0;

		while (m_Running)
		{
			currentTime = glfwGetTime();
			timeDiff = currentTime - prevTime;
			++counter;
			if (timeDiff >= updateRat)
			{
				if (windowIndex >= ROLLING_WINDOW_SIZE)
				{
					windowIndex = 0;
				}

				m_Fps = (1 / timeDiff) * counter;
				m_FrameTime = (timeDiff / counter) * 1000;
				prevTime = currentTime;
				m_FpsWindow[windowIndex] = m_Fps;
				m_FrameTimeWindow[windowIndex] = m_FrameTime;
				m_AverageFrametime = std::reduce(m_FrameTimeWindow.begin(), m_FrameTimeWindow.end()) / ROLLING_WINDOW_SIZE;
				m_AverageFps = std::reduce(m_FpsWindow.begin(), m_FpsWindow.end()) / ROLLING_WINDOW_SIZE;
				++windowIndex;
				counter = 0;
			}

			auto ts = base::Timestep(timer.ElapsedNs());
			timer.Reset();

			OnFrame(ts);
		}
		OnEnd();
#endif
	}

	void Application::OnFrame(base::Timestep ts)
	{
		const base::WindowResizedEvent* lastWindowResizeEvent = nullptr;
		for (const base::Event& ev : m_Window->GetEvents())
		{
			switch (ev.type)
			{
			case  base::EventType::WindowResized:
				lastWindowResizeEvent = &ev.as.windowResizedEvent;
				break;
			case base::EventType::WindowClosed:
				m_Running = false;
				return;
			case base::EventType::KeyPressed:
				{
					if (ev.as.keyPressedEvent.key == GLFW_KEY_R)
					{
						LOG_WARN("Pipeline refresh requested");
						InitializeRenderPipelines();
						p_App->IntializePipeline(m_Builder);
						p_RenderPipeline = m_Builder.BuildPipeline();
					}
					else
					{
						m_Camera.KeyboardEvent(ev.as.keyPressedEvent.key);
					}
				}
			case base::EventType::MouseMoved:
				{
					static glm::vec2 previousPosition = { ev.as.mouseMovedEvent.xPos, ev.as.mouseMovedEvent.yPos };
					glm::vec2 currentPosition = { ev.as.mouseMovedEvent.xPos, ev.as.mouseMovedEvent.yPos };
					const glm::vec2 delta = previousPosition - currentPosition;
					if (m_ShouldRotate)
					{
						m_Camera.Rotate(delta.x, delta.y);
					}
					if (m_ShouldZoom)
					{
						m_Camera.SetZoomDistance(-delta.y);
					}
					previousPosition = currentPosition;
					break;
				}
			case base::EventType::MousePressed:
				{
					if (!ImGui::GetIO().WantCaptureMouse)
					{
						ToggleMouse(ev.as.mousePressedEvent.button, true);
					}
				break;
				}
			case base::EventType::MouseReleased:
				{
					ToggleMouse(ev.as.mouseReleasedEvent.button, false);
					break;
				}
			default:
				break;
			}
		}

		if (lastWindowResizeEvent)
		{
			LOG_TRACE("Resizing window to: ", lastWindowResizeEvent->width, "x", lastWindowResizeEvent->height);
			OnResize(lastWindowResizeEvent->width, lastWindowResizeEvent->height);
		}

		OnUpdate(ts);

		if (m_Window->GetWidth() != 0 && m_Window->GetHeight() != 0)
		{
			OnRender();
		}

		m_Window->Update();
		wgpuDeviceTick(base::GraphicsContext::GetDevice());
	}

#if defined(PLATFORM_WEB)
	EM_BOOL Application::EMSRedraw(double time, void* userData)
	{
		if (userData == nullptr)
		{
			return false;
		}

		Application* app = static_cast<Application*>(userData);
		app->OnFrame(Timestep(time));
		return true;
	}
#endif

	/* Helpers */

	void Application::InitializeSamplers()
	{
		LOG_INFO("Initializing samplers");
		p_Sampler = Sampler::CreateSampler(base::GraphicsContext::GetDevice(), WGPUFilterMode_Linear, WGPUMipmapFilterMode_Linear);
		p_SamplerNN = Sampler::CreateSampler(base::GraphicsContext::GetDevice(), WGPUFilterMode_Nearest, WGPUMipmapFilterMode_Nearest);
	}

	void Application::InitializeUniforms()
	{
		LOG_INFO("Initializing default Application uniforms");

		const auto device = base::GraphicsContext::GetDevice();
		const auto queue = base::GraphicsContext::GetQueue();

		glm::mat4 dummy_model{ 1.0f };
		constexpr float UNIFORM_CAMERA_SIZE = sizeof(float) * 16 * 5; // float * mat4 *  card({Model, View, Proj, InverseProj, InverseView})
		p_UCamera = UniformBuffer::CreateFromData(device, queue, static_cast<void*>(&dummy_model),
			/* float * mat4 *  MVP IP IV = */ sizeof(float) * 16 * 5, 0, false, "Camera Uniform");		
		p_UCameraPos =	UniformBuffer::CreateFromData(device, queue, glm::value_ptr(m_Camera.GetPosition()), sizeof(glm::vec3));
		p_UFragmentMode = UniformBuffer::CreateFromData(device, queue, &m_FragmentMode, sizeof(int));
		p_UStepsCount = UniformBuffer::CreateFromData(device, queue, &m_StepsCount, sizeof(int));
		p_UStepSize = UniformBuffer::CreateFromData(device, queue, &m_StepSize, sizeof(float));

		p_UClipX = UniformBuffer::CreateFromData(device, queue, glm::value_ptr(m_ClipsX), sizeof(glm::vec2));
		p_UClipY = UniformBuffer::CreateFromData(device, queue, glm::value_ptr(m_ClipsY), sizeof(glm::vec2));
		p_UClipZ = UniformBuffer::CreateFromData(device, queue, glm::value_ptr(m_ClipsZ), sizeof(glm::vec2));
		p_UToggles= UniformBuffer::CreateFromData(device, queue, glm::value_ptr(m_Toggles), sizeof(glm::ivec4));


		p_UCamera->UpdateBuffer(queue, sizeof(float) * 16, &m_Camera.GetViewMatrix(), sizeof(float) * 16);
		p_UCamera->UpdateBuffer(queue, sizeof(float) * 16 * 2, &m_Camera.GetProjectionMatrix(), sizeof(float) * 16);
		p_UCamera->UpdateBuffer(queue, sizeof(float) * 16 * 3, &m_Camera.GetInverseViewMatrix(), sizeof(float) * 16);
		p_UCamera->UpdateBuffer(queue, sizeof(float) * 16 * 4, &m_Camera.GetInverseProjectionMatrix(), sizeof(float) * 16);
	}

	void Application::InitializeTextures()
	{
		LOG_INFO("Initializing proxy-geometry render attachments");
		p_TexEndPos = Texture::CreateRenderAttachment(m_Width, m_Height, WGPUTextureUsage_TextureBinding, "Back Faces Texture");
	}

	void Application::InitializeVertexBuffers()
	{
		LOG_INFO("Initializing proxy geometry vertex buffer");
		p_VBCube = VertexBuffer::CreateFromData(base::GraphicsContext::GetDevice(), base::GraphicsContext::GetQueue(), m_CubeVertexData, sizeof(m_CubeVertexData), 0, false, "Cube VertexBuffer");
		//. XYZ
		p_VBCube->AddVertexAttribute({
			.format = WGPUVertexFormat_Float32x3,
			.offset = 0,
			.shaderLocation = 0
		});
		//. UVW
		p_VBCube->AddVertexAttribute({
			.format = WGPUVertexFormat_Float32x3,
			.offset = sizeof(float) * 3,
			.shaderLocation = 1
		});
	}

	void Application::InitializeIndexBuffers()
	{
		LOG_INFO("Initializing proxy geometry index buffer");
		p_IBCube = IndexBuffer::CreateFromData(base::GraphicsContext::GetDevice(), base::GraphicsContext::GetQueue(), m_CubeIndexData,
			sizeof(m_CubeIndexData), sizeof(m_CubeIndexData) / sizeof(uint16_t));
	}

	void Application::InitializeBindGroups()
	{
		LOG_INFO("Initializing default Application BindGroup");
		m_BGroupDefaultApp = BindGroup();
		m_BGroupDefaultApp.AddBuffer(*p_UCamera, WGPUShaderStage_Vertex | WGPUShaderStage_Fragment);
		m_BGroupDefaultApp.AddBuffer(*p_UCameraPos, WGPUShaderStage_Vertex | WGPUShaderStage_Fragment);
		m_BGroupDefaultApp.AddSampler(*p_Sampler);
		m_BGroupDefaultApp.AddSampler(*p_SamplerNN);
		m_BGroupDefaultApp.AddBuffer(*p_UFragmentMode, WGPUShaderStage_Fragment);
		m_BGroupDefaultApp.AddBuffer(*p_UStepsCount, WGPUShaderStage_Fragment);
		m_BGroupDefaultApp.AddBuffer(*p_UStepSize, WGPUShaderStage_Fragment);
		m_BGroupDefaultApp.AddBuffer(*p_UClipX, WGPUShaderStage_Fragment);
		m_BGroupDefaultApp.AddBuffer(*p_UClipY, WGPUShaderStage_Fragment);
		m_BGroupDefaultApp.AddBuffer(*p_UClipZ, WGPUShaderStage_Fragment);
		m_BGroupDefaultApp.AddBuffer(*p_UToggles, WGPUShaderStage_Fragment);
		m_BGroupDefaultApp.AddTexture(*p_TexEndPos, WGPUShaderStage_Fragment, WGPUTextureSampleType_UnfilterableFloat);
		m_BGroupDefaultApp.FinalizeBindGroup(base::GraphicsContext::GetDevice());

		m_BGroupProxy = BindGroup();
		m_BGroupProxy.AddBuffer(*p_UCamera, WGPUShaderStage_Vertex | WGPUShaderStage_Fragment);
		m_BGroupProxy.AddBuffer(*p_UCameraPos, WGPUShaderStage_Vertex | WGPUShaderStage_Fragment);
		m_BGroupProxy.FinalizeBindGroup(base::GraphicsContext::GetDevice());
	}

	void Application::InitializeRenderPipelines()
	{
		LOG_INFO("Initializing render pipelines");

		// Main pipeline camera, basic ui
		{
			m_Builder = PipelineBuilder();
			m_Builder.DepthTexMagic(m_Width, m_Height);
			m_Builder.AddBuffer(*p_VBCube);
			m_Builder.AddBindGroup(m_BGroupDefaultApp);
			m_Builder.SetFrontFace(WGPUFrontFace_CCW);
			m_Builder.SetCullFace(WGPUCullMode_Back);
		}

		// proxy pipelines
		{
			WGPUShaderModule shaderModuleAtt = Shader::create_shader_module(base::GraphicsContext::GetDevice(),
				FileSystem::ReadFile(FileSystem::GetDefaultPath() / "shaders" / "rayCoords.wgsl"));

			PipelineBuilder builderAtt;
			builderAtt.AddBuffer(*p_VBCube);
			builderAtt.AddBindGroup(m_BGroupProxy);
			builderAtt.AddShaderModule(shaderModuleAtt);
			builderAtt.SetFrontFace(WGPUFrontFace_CCW);
			builderAtt.SetColorTargetFormat(WGPUTextureFormat_RGBA32Float);
			builderAtt.SetCullFace(WGPUCullMode_Front);
			p_RenderPipelineEnd = builderAtt.BuildPipeline();
		}

		{
			WGPUShaderModule shaderModuleAtt = Shader::create_shader_module(base::GraphicsContext::GetDevice(),
				FileSystem::ReadFile(FileSystem::GetDefaultPath() / "shaders" / "fullscreen.wgsl"));
			PipelineBuilder builderBackground;
			builderBackground.AddShaderModule(shaderModuleAtt);
			p_RenderPipelineBackground = builderBackground.BuildPipeline();
		}
	}

	void Application::ToggleMouse(int key, bool toggle)
	{
		switch (key)
		{
		case GLFW_MOUSE_BUTTON_LEFT:
			m_ShouldRotate = toggle;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			m_ShouldZoom = toggle;
			break;
		default:
			break;
		}
	}

}
