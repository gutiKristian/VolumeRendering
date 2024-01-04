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

#include <memory>

#include "dicom/VolumeFile.h"

int main(int argc, char* argv[]);

namespace med {

	class Application
	{
	public:
		Application();
		~Application();
		/*
		 * Application data initialization
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
		Camera m_Camera = Camera::CreatePerspective(glm::radians(60.0f), static_cast<float>(m_Width) / static_cast<float>(m_Height), 0.1f, 100.0f);
		glm::vec2 m_MousePos{};
		bool m_ShouldRotate = false;
		bool m_ShouldZoom = false;
		//

		std::shared_ptr<UniformBuffer> p_UCamera = nullptr;
		std::shared_ptr<UniformBuffer> p_UCameraPos = nullptr;
		std::shared_ptr<UniformBuffer> p_UFragmentMode = nullptr;

		std::shared_ptr<Texture> p_TexData = nullptr;
		std::shared_ptr<Texture> p_TexStart = nullptr;
		std::shared_ptr<Texture> p_TexEnd = nullptr;


		std::shared_ptr<RenderPipeline> p_RenderPipeline = nullptr;
		std::shared_ptr<RenderPipeline> p_RenderPipelineStart = nullptr;
		std::shared_ptr<RenderPipeline> p_RenderPipelineEnd = nullptr;

		BindGroup m_BGroupCamera;
		BindGroup m_BGroupTextures;
		BindGroup m_BGroupImGui;

		std::shared_ptr<VertexBuffer> p_VBCube = nullptr;
		std::shared_ptr<IndexBuffer> p_IBCube = nullptr;

		std::shared_ptr<Sampler> p_Sampler = nullptr;
		// ----------------------------------------- 
		float m_CubeVertexData[48] = {
			/*,	   x     y      z     u	  v	   w */
				 -1.0, -1.0,  0.5,	0.0, 0.0, 0.0,
				  1.0, -1.0,  0.5,	1.0, 0.0, 0.0,
				 -1.0,  1.0,  0.5,  0.0, 1.0, 0.0,
				  1.0,  1.0,  0.5,  1.0, 1.0, 0.0,
				 -1.0, -1.0, -0.5,  0.0, 0.0, 1.0,
				  1.0, -1.0, -0.5,  1.0, 0.0, 1.0,
				 -1.0,  1.0, -0.5,  0.0, 1.0, 1.0,
				  1.0,  1.0, -0.5,  1.0, 1.0, 1.0
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
