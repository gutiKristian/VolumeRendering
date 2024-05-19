#pragma once

#include "Base/Window.h"
#include "Base/Timestep.h"
#include "Camera.h"
#include "renderer/VertexBuffer.h"
#include "renderer/UniformBuffer.h"
#include "renderer/BindGroup.h"
#include "renderer/PipelineBuilder.h"
#include "renderer/RenderPipeline.h"
#include "renderer/Texture.h"
#include "renderer/IndexBuffer.h"
#include "tf/OpacityTf.h"
#include "tf/ColorTf.h"
#include "file/VolumeFile.h"
#include "miniapps/include/MiniApp.h"

#include <memory>

int main(int argc, char* argv[]);

namespace med {

	class Application
	{
	public:
		Application();
		~Application();
		/*
		 * Application, default primitives initialization
		 * Calls MiniApp initialization as well
		*/
		void OnStart();
		/*
		 * Updating data
		 */
		void OnUpdate(base::Timestep ts);
		/*
		 * Render calls.
		 */
		void OnRender();
		/*
		 * Rendering UI.
		 */
		void OnImGuiRender();
		/*
		 * Recreates main pipeline.
		 */
		void OnResize(uint32_t width, uint32_t height);
		/*
		 * Called when application is closed (not in ems for now)
		 */
		void OnEnd();
	private:
		/*
		 * Main loop of the application
		 */
		void Run();
		/*
		 * Event processing stuff and pre-precessing things around window
		 */
		void OnFrame(base::Timestep ts);
		/* Initializers */
		void InitializeSamplers();
		void InitializeUniforms();
		void InitializeTextures();
		void InitializeVertexBuffers();
		void InitializeIndexBuffers();
		void InitializeBindGroups();

		/*
		* Set the size of the bounding box. Default is a unit cube.
		* The paramters are sizes (multiplied).
		* @param x: size on the x axis
		* @param y: size on the y axis
		* @param z: size on the z axis
		*/
		void SetBoundingBoxSize(float x, float y, float z);

		/*
		* Initializes pipelines that are used during the runtime
		* This function is called also on resize when pipelines need to be rebuilt
		*/
		void InitializeRenderPipelines();

		/*Event helpers*/
		void ToggleMouse(int key, bool toggle);

	private:
#if defined(PLATFORM_WEB)
		static int EMSRedraw(double time, void* userData);
#endif
	private:
		base::Window* m_Window;
		bool m_Running = false;
	private:
		friend int ::main(int argc, char* argv[]);
	private:
		// data
		uint32_t m_Width = 1280;
		uint32_t m_Height = 720;

		// Camera utils
		Camera m_Camera = Camera::CreatePerspective(glm::radians(60.0f), static_cast<float>(m_Width) / static_cast<float>(m_Height), 0.01f, 100.0f);
		glm::vec2 m_MousePos{};
		bool m_ShouldRotate = false;
		bool m_ShouldZoom = false;
		
		glm::vec2 m_ClipsX{ 0.0f };
		glm::vec2 m_ClipsY{ 0.0f };
		glm::vec2 m_ClipsZ{ 0.0f };

		// Main Pipeline
		PipelineBuilder m_Builder;

		// Binding 0
		std::shared_ptr<UniformBuffer> p_UCamera = nullptr;
		std::shared_ptr<UniformBuffer> p_UCameraPos = nullptr;
		std::shared_ptr<UniformBuffer> p_UClipX = nullptr;
		std::shared_ptr<UniformBuffer> p_UClipY = nullptr;
		std::shared_ptr<UniformBuffer> p_UClipZ = nullptr;
		std::shared_ptr<UniformBuffer> p_UToggles = nullptr;


		std::shared_ptr<Texture> p_TexEndPos = nullptr;

		std::shared_ptr<UniformBuffer> p_UFragmentMode = nullptr;
		std::shared_ptr<UniformBuffer> p_UStepsCount = nullptr;
		std::shared_ptr<UniformBuffer> p_UStepSize = nullptr;

		std::shared_ptr<RenderPipeline> p_RenderPipeline = nullptr;
		std::shared_ptr<RenderPipeline> p_RenderPipelineEnd = nullptr;
		std::shared_ptr<RenderPipeline> p_RenderPipelineBackground = nullptr;

		BindGroup m_BGroupDefaultApp;
		BindGroup m_BGroupProxy;
		
		std::shared_ptr<VertexBuffer> p_VBCube = nullptr;
		std::shared_ptr<IndexBuffer> p_IBCube = nullptr;

		std::shared_ptr<Sampler> p_Sampler = nullptr;
        std::shared_ptr<Sampler> p_SamplerNN = nullptr;

		std::unique_ptr<MiniApp> p_App = nullptr;

		// -----------------------------------------
		float m_CubeVertexData[48] = {
			/*,	   x     y      z     u	  v	   w */
				 -0.5f, -0.5f,  0.25f,	0.0f, 0.0f, 0.0f,
				  0.5f, -0.5f,  0.25f,	1.0f, 0.0f, 0.0f,
				 -0.5f,  0.5f,  0.25f,	0.0f, 1.0f, 0.0f,
				  0.5f,  0.5f,  0.25f,	1.0f, 1.0f, 0.0f,
				 -0.5f, -0.5f, -0.25f,	0.0f, 0.0f, 1.0f,
				  0.5f, -0.5f, -0.25f,	1.0f, 0.0f, 1.0f,
				 -0.5f,  0.5f, -0.25f,	0.0f, 1.0f, 1.0f,
				  0.5f,  0.5f, -0.25f,	1.0f, 1.0f, 1.0f
		};

		uint16_t m_CubeIndexData[36] = {
			1, 5, 3, 5, 7, 3,
			0, 4, 1, 1, 4, 5,
			2, 3, 6, 3, 7, 6,
			4, 6, 5, 5, 6, 7,
			4, 0, 6, 0, 2, 6,
			0, 1, 2, 1, 3, 2
		};

		// Mouse handlers
		bool m_IsLeftClicked = false;
		bool m_IsRightClicked = false;
		float m_LastX = 0.0f;
		float m_LastY = 0.0f;
	
		// ImGui
		int m_FragmentMode = 0;
		int m_StepsCount = 200;
		float m_StepSize = 0.01f;
		float m_FrameTime = 0.0f;
		float m_Fps = 0.0;
		float m_AverageFrametime = 0.0;
		float m_AverageFps = 0.0;
		const int ROLLING_WINDOW_SIZE = 120;
		std::vector<float> m_FpsWindow;
		std::vector<float> m_FrameTimeWindow;


		// (variable step size, jitter, -, -)
		bool m_BToggles[4] = { false, false, false, false };
		glm::ivec4 m_Toggles{ 0 };

		const char* m_FragModes[5] =
		{
			"Volume",
			"Ray direction",
			"Front faces",
			"Back Faces",
			"Texture coordinates"
		};
	};

}
