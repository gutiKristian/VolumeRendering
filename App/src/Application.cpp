#include "Application.h"

#include "Base/Base.h"
#include "Base/Filesystem.h"
#include "Base/Timer.h"

#include "Base/GraphicsContext.h"

#include <webgpu/webgpu_cpp.h>

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

		base::WindowProps props{ .width = 1280, .height = 720, .title = "Volume Rendering" };
		m_Window = new base::Window(props);
	}

	Application::~Application()
	{
		delete m_Window;
	}

	void Application::OnUpdate(base::Timestep ts)
	{
	}

	void Application::OnRender()
	{
	}

	void Application::OnImGuiRender()
	{
	}

	void Application::Run()
	{
		INFO("Run Application");
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
			}
		}

		OnUpdate(ts);

		if (m_Window->GetWidth() != 0 && m_Window->GetHeight() != 0)
		{
			OnRender();
		}

		m_Window->Update();
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

}
