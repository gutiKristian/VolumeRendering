#include"Application.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <webgpu/webgpu_cpp.h>

#include "Base/Base.h"
#include "Base/Filesystem.h"
#include "Base/Timer.h"
#include "Base/GraphicsContext.h"

#include "dicom/FileReader.h"
#include "dicom/DcmImpl.h"
#include "dicom/DatImpl.h"

#include "Shader.h"
#include "ImGuiLayer.h"
#include "implot.h"

#include "tf/LinearInterpolation.h"

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
		InitializeTransferFunction();
		InitializeVertexBuffers();
		InitializeIndexBuffers();
		InitializeBindGroups();
		InitializeRenderPipelines();
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

		if (m_ShouldUpdateTf)
		{
			m_ShouldUpdateTf = false;
			LOG_INFO("TF update initiated");
			p_TexTf->UpdateTexture(base::GraphicsContext::GetQueue(), m_TfY);
		}
	}

	void Application::OnRender()
	{
		const WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(base::GraphicsContext::GetDevice(), nullptr);

		// Ray entrance

		WGPURenderPassColorAttachment colorAttachmentsRay = {
			.view = p_TexStart->GetTextureView(),
			.loadOp = WGPULoadOp_Clear,
			.storeOp = WGPUStoreOp_Store,
			.clearValue = {0.0, 0.0, 0.0, 1.0}
		};

		WGPURenderPassDescriptor renderPassDescRay = {
			.label = "Ray entrance render pass",
			.colorAttachmentCount = 1,
			.colorAttachments = &colorAttachmentsRay,
			.depthStencilAttachment = nullptr
		};

		const WGPURenderPassEncoder passRayEntrance = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDescRay);

		p_RenderPipelineStart->Bind(passRayEntrance);
		p_VBCube->Bind(passRayEntrance);
		m_BGroupCamera.Bind(passRayEntrance);
		wgpuRenderPassEncoderSetIndexBuffer(passRayEntrance, p_IBCube->GetBufferPtr(), WGPUIndexFormat_Uint16, 0, p_IBCube->GetSize());
		wgpuRenderPassEncoderDrawIndexed(passRayEntrance, p_IBCube->GetCount(), 1, 0, 0, 0);
		wgpuRenderPassEncoderEnd(passRayEntrance);
		BindGroup::ResetBindSlotsIndices();

		// Ray End
		colorAttachmentsRay.view = p_TexEnd->GetTextureView();
		renderPassDescRay.label = "Ray end render pass";

		const WGPURenderPassEncoder passRayEnd = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDescRay);

		p_RenderPipelineEnd->Bind(passRayEnd);
		p_VBCube->Bind(passRayEnd);
		m_BGroupCamera.Bind(passRayEnd);
		wgpuRenderPassEncoderSetIndexBuffer(passRayEnd, p_IBCube->GetBufferPtr(), WGPUIndexFormat_Uint16, 0, p_IBCube->GetSize());
		wgpuRenderPassEncoderDrawIndexed(passRayEnd, p_IBCube->GetCount(), 1, 0, 0, 0);
		wgpuRenderPassEncoderEnd(passRayEnd);
		BindGroup::ResetBindSlotsIndices();

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
			.loadOp = WGPULoadOp_Clear,
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
		m_BGroupCamera.Bind(pass);
		m_BGroupTextures.Bind(pass);
		m_BGroupImGui.Bind(pass);
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
		ImGui::Begin("Fragment Mode");
		ImPlot::ShowDemoWindow();
		ImGui::ListBox("##", &m_FragmentMode, m_FragModes, 5);
		ImGui::SliderInt("Number of steps", &m_StepsCount, 0, 1500);
		ImGui::End();

        OnTfRender();
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

		// Reinitialize first pass render attachments
		p_TexStart = Texture::CreateRenderAttachment(width, height, WGPUTextureUsage_TextureBinding, "Front Faces Texture");
		p_TexEnd = Texture::CreateRenderAttachment(width, height, WGPUTextureUsage_TextureBinding, "Back Faces Texture");

		// Reinitialize bind group
		m_BGroupTextures = BindGroup();
		m_BGroupTextures.AddTexture(*p_TexData, WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroupTextures.AddTexture(*p_TexStart, WGPUShaderStage_Fragment, WGPUTextureSampleType_UnfilterableFloat);
		m_BGroupTextures.AddTexture(*p_TexEnd, WGPUShaderStage_Fragment, WGPUTextureSampleType_UnfilterableFloat);
		m_BGroupTextures.AddSampler(*p_Sampler);
		m_BGroupTextures.AddTexture(*p_TexTf, WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroupTextures.FinalizeBindGroup(base::GraphicsContext::GetDevice());
		
		// Reinit pipelines
		InitializeRenderPipelines();
	}

	void Application::OnEnd()
	{
		LOG_INFO("On end");
		ImGuiLayer::Destroy();
	}

	void Application::Run()
	{
		LOG_INFO("Run Application");
		m_Running = true;
#if defined(PLATFORM_WEB)
		emscripten_request_animation_frame_loop(Application::EMSRedraw, (void*)this);
#else
		base::Timer timer;
		while (m_Running)
		{
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
				m_Camera.KeyboardEvent(ev.as.keyPressedEvent.key);
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
		p_Sampler = Sampler::CreateSampler(base::GraphicsContext::GetDevice());
	}

	void Application::InitializeUniforms()
	{
		LOG_INFO("Initializing uniforms");
		glm::mat4 dummy_model{ 1.0f };
		constexpr float UNIFORM_CAMERA_SIZE = sizeof(float) * 16 * 5; // float * mat4 *  card({Model, View, Proj, InverseProj, InverseView})
		p_UCamera = UniformBuffer::CreateFromData(base::GraphicsContext::GetDevice(), base::GraphicsContext::GetQueue(), static_cast<void*>(&dummy_model),
			/* float * mat4 *  MVP IP IV = */ sizeof(float) * 16 * 5, 0, false, "Camera Uniform");
		p_UCamera->UpdateBuffer(base::GraphicsContext::GetQueue(), sizeof(float) * 16, &m_Camera.GetViewMatrix(), sizeof(float) * 16);
		p_UCamera->UpdateBuffer(base::GraphicsContext::GetQueue(), sizeof(float) * 16 * 2, &m_Camera.GetProjectionMatrix(), sizeof(float) * 16);
		p_UCamera->UpdateBuffer(base::GraphicsContext::GetQueue(), sizeof(float) * 16 * 3, &m_Camera.GetInverseViewMatrix(), sizeof(float) * 16);
		p_UCamera->UpdateBuffer(base::GraphicsContext::GetQueue(), sizeof(float) * 16 * 4, &m_Camera.GetInverseProjectionMatrix(), sizeof(float) * 16);


		p_UCameraPos = UniformBuffer::CreateFromData(base::GraphicsContext::GetDevice(), base::GraphicsContext::GetQueue(), glm::value_ptr(m_Camera.GetPosition()), sizeof(glm::vec3));
		p_UFragmentMode = UniformBuffer::CreateFromData(base::GraphicsContext::GetDevice(), base::GraphicsContext::GetQueue(), &m_FragmentMode, sizeof(int));
		p_UStepsCount = UniformBuffer::CreateFromData(base::GraphicsContext::GetDevice(), base::GraphicsContext::GetQueue(), &m_StepsCount, sizeof(int));
	}

	void Application::InitializeTextures()
	{
		LOG_INFO("Initializing textures");
		DcmImpl reader;
		DatImpl reader2;
		VolumeFile file = reader.readFile("assets\\716^716_716_CT_2013-04-02_230000_716-1-01_716-1_n81__00000", true);
		//VolumeFile file2 = reader2.readFile("assets\\stagbeetle277x277x164.dat", false);

		p_TexData = Texture::CreateFromData(base::GraphicsContext::GetDevice(), base::GraphicsContext::GetQueue(), file.get4BPtr(), WGPUTextureDimension_3D, file.getSize(),
			WGPUTextureFormat_R32Float, WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst, sizeof(float), "Data texture");

		p_TexStart = Texture::CreateRenderAttachment(m_Width, m_Height, WGPUTextureUsage_TextureBinding, "Front Faces Texture");
		p_TexEnd = Texture::CreateRenderAttachment(m_Width, m_Height, WGPUTextureUsage_TextureBinding, "Back Faces Texture");
	}

	void Application::InitializeVertexBuffers()
	{
		LOG_INFO("Initializing vertex buffers");
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
		LOG_INFO("Initializing index buffers");
		p_IBCube = IndexBuffer::CreateFromData(base::GraphicsContext::GetDevice(), base::GraphicsContext::GetQueue(), m_CubeIndexData,
			sizeof(m_CubeIndexData), sizeof(m_CubeIndexData) / sizeof(uint16_t));
	}

	void Application::InitializeBindGroups()
	{
		LOG_INFO("Initializing bind groups");
		m_BGroupCamera.AddBuffer(*p_UCamera, WGPUShaderStage_Vertex | WGPUShaderStage_Fragment);
		m_BGroupCamera.AddBuffer(*p_UCameraPos, WGPUShaderStage_Vertex | WGPUShaderStage_Fragment);
		m_BGroupCamera.FinalizeBindGroup(base::GraphicsContext::GetDevice());

		m_BGroupTextures.AddTexture(*p_TexData, WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroupTextures.AddTexture(*p_TexStart, WGPUShaderStage_Fragment, WGPUTextureSampleType_UnfilterableFloat);
		m_BGroupTextures.AddTexture(*p_TexEnd, WGPUShaderStage_Fragment, WGPUTextureSampleType_UnfilterableFloat);
		m_BGroupTextures.AddSampler(*p_Sampler);
		m_BGroupTextures.AddTexture(*p_TexTf, WGPUShaderStage_Fragment, WGPUTextureSampleType_Float);
		m_BGroupTextures.FinalizeBindGroup(base::GraphicsContext::GetDevice());

		m_BGroupImGui.AddBuffer(*p_UFragmentMode, WGPUShaderStage_Fragment);
		m_BGroupImGui.AddBuffer(*p_UStepsCount, WGPUShaderStage_Fragment);
		m_BGroupImGui.FinalizeBindGroup(base::GraphicsContext::GetDevice());
	}

	void Application::InitializeRenderPipelines()
	{
		LOG_INFO("Initializing render pipelines");
		//Hardcode for now
		FileReader shaderReader;
		shaderReader.setDefaultPath(shaderReader.getDefaultPath() / "shaders");

		WGPUShaderModule shaderModule = Shader::create_shader_module(base::GraphicsContext::GetDevice(), shaderReader.readFile("simple.wgsl"));


		PipelineBuilder builder;
		builder.DepthTexMagic(m_Width, m_Height);
		builder.AddBuffer(*p_VBCube);
		builder.AddBindGroup(m_BGroupCamera);
		builder.AddBindGroup(m_BGroupTextures);
		builder.AddBindGroup(m_BGroupImGui);
		builder.AddShaderModule(shaderModule);
		builder.SetFrontFace(WGPUFrontFace_CCW);
		builder.SetCullFace(WGPUCullMode_Back);
		//builder.SetCullFace(WGPUCullMode_Front);
		p_RenderPipeline = builder.BuildPipeline();

		WGPUShaderModule shaderModuleAtt = Shader::create_shader_module(base::GraphicsContext::GetDevice(), shaderReader.readFile("rayCoords.wgsl"));

		PipelineBuilder builderAtt;
		builderAtt.AddBuffer(*p_VBCube);
		builderAtt.AddBindGroup(m_BGroupCamera);
		builderAtt.AddShaderModule(shaderModuleAtt);
		builderAtt.SetFrontFace(WGPUFrontFace_CCW);
		builderAtt.SetColorTargetFormat(WGPUTextureFormat_RGBA32Float);

		builderAtt.SetCullFace(WGPUCullMode_Back);
		p_RenderPipelineStart = builderAtt.BuildPipeline();

		builderAtt.SetCullFace(WGPUCullMode_Front);
		p_RenderPipelineEnd = builderAtt.BuildPipeline();

	}

	void Application::InitializeTransferFunction()
	{
		LOG_INFO("Initializing transfer function");

		// We are working with 12bit data
        m_TfContrPHandle.emplace_back(0.0, 0.0);
        m_TfContrPHandle.emplace_back(4095.0, 1.0);

		std::vector<float> result = LinearInterpolation::Generate<float>(0, 4095, 0.0f, 1.0f, 1);

		assert(result.size() == 4096 && "Size of generated TF does not match");

		// Fill for plotting
		for (size_t i = 0; i < 4096; ++i)
		{
			m_TfX[i] = i;
			m_TfY[i] = result[i];
		}

		p_TexTf = Texture::CreateFromData(base::GraphicsContext::GetDevice(), base::GraphicsContext::GetQueue(), m_TfY, WGPUTextureDimension_1D, {4096, 1, 1},
			WGPUTextureFormat_R32Float, WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst, sizeof(float), "Transfer function");
	
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

    size_t Application::AddControlPoint(double x, double y)
    {
        // On x-axis we always work with nearest integer values -- from 0 to 'bit depth of the voxel data'
        x = std::round(x);

        // Retrieve the index of this point if already exist
        auto iter = std::find_if(m_TfContrPHandle.begin(), m_TfContrPHandle.end(), [x] (const auto& obj){ return obj.x == x; });
        if (iter != m_TfContrPHandle.end())
        {
            LOG_TRACE("Control point already exists, drag it to edit its value");
            return -1;
        }

        // Create Implot handles for control points
        m_TfContrPHandle.emplace_back(x, y);
        // Helps us recalculate data between the neighbourhood points
        std::ranges::sort(m_TfContrPHandle, [](const auto& a, const auto& b) {return a.x < b.x; });

        //TODO: what the distance returns when two end matches
        iter = std::find_if(m_TfContrPHandle.begin(), m_TfContrPHandle.end(), [x] (const auto& obj){ return obj.x == x; });

        LOG_TRACE("Added new point");
        return std::distance(m_TfContrPHandle.begin(), iter);
    }

    void Application::UpdateTfDataIntervals(size_t controlPointIndex)
    {
        // Takes x and y coordinate of two control points and execute linear interpolation between them, then copies to resulting array
        auto updateIntervalValues = [&](double cx1, double cx2, float cy1, float cy2)
        {
            int x0 = static_cast<int>(cx1);
            int x1 = static_cast<int>(cx2);

            std::vector<float> result = LinearInterpolation::Generate<float>(x0, x1, cy1, cy2, 1);
            assert(result.size() - 1 == std::abs(x1 - x0) && "Size of generated vector does not match");

            for (size_t i = 0; i < std::abs(x1 - x0); ++i)
            {
                m_TfY[i + x0] = result[i];
            }
        };

        // Update control interval between control point below and current
        if (controlPointIndex - 1 >= 0)
        {
            auto predecessorIndex = static_cast<size_t>(m_TfContrPHandle[controlPointIndex-1].x);
            updateIntervalValues(m_TfContrPHandle[controlPointIndex - 1].x, m_TfContrPHandle[controlPointIndex].x,
                                 m_TfY[predecessorIndex], static_cast<float>(m_TfContrPHandle[controlPointIndex].y));
        }

        // Update control interval between current control point and control point above
        if (controlPointIndex + 1 < m_TfContrPHandle.size())
        {
            auto successorIndex = static_cast<size_t>(m_TfContrPHandle[controlPointIndex+1].x);
            updateIntervalValues(m_TfContrPHandle[controlPointIndex].x, m_TfContrPHandle[controlPointIndex + 1].x,
                                 static_cast<float>(m_TfContrPHandle[controlPointIndex].y), m_TfY[successorIndex]);
        }
    }

    void Application::OnTfRender()
    {
        if (ImPlot::BeginPlot("Transfer function"))
        {
            ImPlot::SetupAxes("Voxel value", "Alpha");

            // Setup limits, X: 0-4095 (data resolution -- bits per pixel), Y: 0-1 (could be anything in the future)
            ImPlot::SetupAxisLimitsConstraints(ImAxis_X1,0.0, 4095.0);
            ImPlot::SetupAxisLimitsConstraints(ImAxis_Y1,0.0, 1.0);

            bool isDragging = false;
            bool hasClicked = false;
            int draggedId = -1;

            // Create drag points
            for (int id = 0; id < m_TfContrPHandle.size(); ++id)
            {
                isDragging |= ImPlot::DragPoint(id, &m_TfContrPHandle[id].x, &m_TfContrPHandle[id].y,
                        ImVec4(0,0.9f,0,1), 4, ImPlotDragToolFlags_None, nullptr, nullptr, nullptr);

                if (isDragging && draggedId == -1)
                {
                    // This point is being dragged and we will work with it below
                    draggedId = id;
                    CheckDragBounds(draggedId);
                }
            }

            ImPlot::PlotLine("Tf", m_TfX, m_TfY, 4096);

            // Drag event
            if (isDragging)
            {
                UpdateTfDataIntervals(draggedId);
                m_ShouldUpdateTf = true;
                ImPlot::EndPlot();
                return;
            }

            auto dragDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 0.1f);
            hasClicked = ImPlot::IsPlotHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left)
                         && dragDelta.x == 0.0 && dragDelta.y == 0.0;

            // Click event
            if (hasClicked)
            {
                ImPlotPoint mousePos = ImPlot::GetPlotMousePos();

                std::stringstream ss;
                ss << "Clicked in plot:" << "\tx: " << static_cast<int>(mousePos.x) << " | y: " << mousePos.y;
                LOG_TRACE(ss.str().c_str());

                size_t index = AddControlPoint(mousePos.x, mousePos.y);
                UpdateTfDataIntervals(index);
                ImPlot::EndPlot();
                m_ShouldUpdateTf = true;
                return;
            }

            ImPlot::EndPlot();

        }
    }

    void Application::CheckDragBounds(size_t index)
    {
        constexpr double X_MAX = 4095.0; // Depends on input data depth
        constexpr double X_MIN = 0.0;

        // We must use floor in order to have at least one unit padding between two cps
        auto& cp = m_TfContrPHandle[index];
        const auto cpSize = m_TfContrPHandle.size();

        // Within max y coords
        cp.y = std::clamp(cp.y, 0.0, 1.0);
        cp.x = std::clamp(cp.x, X_MIN, X_MAX);

        // is bounded from left
        if (index - 1 >= 0 && m_TfContrPHandle[index - 1].x >= cp.x)
        {
            cp.x = std::round(m_TfContrPHandle[index - 1].x + 1.0);
        }
        // is bounded from right
        if (index + 1 < cpSize && m_TfContrPHandle[index + 1].x <= cp.x)
        {
            cp.x = std::round(m_TfContrPHandle[index + 1].x - 1.0);
        }

    }
}
