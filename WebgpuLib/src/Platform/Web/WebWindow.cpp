#if !defined(PLATFORM_WEB)
	#error This file should only be compiled for web/emscripten build
#endif

#include "Base/Base.h"
#include "Base/Window.h"
#include "Base/GraphicsContext.h"

#include <GLFW/glfw3.h>

#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/html5_webgpu.h>

#include <iostream>
#include <cassert>

namespace Base {

	EM_JS(void, EMSSetWindowTitle, (const char* title),
	{
		document.title = UTF8ToString(title);
	});

	EM_JS(void, EMSSetWindowHandle, (GLFWwindow* windowHandle),
	{
		const canvas = document.getElementById('canvas');
		canvas.dataset.windowHandle = windowHandle;
	});

	KEEP_IN_MODULE void EMSOnCanvasResized(GLFWwindow* windowHandle, int width, int height)
	{
		glfwSetWindowSize(windowHandle, width, height);
	}

	EM_JS(void, EMSSetResizeCallback, (),
		{
			const canvas = document.getElementById('canvas');
			function OnCanvasResized()
			{
				const width = canvas.getBoundingClientRect().width;
				const height = canvas.getBoundingClientRect().height;
				const windowHandle = canvas.dataset.windowHandle;
				_EMSOnCanvasResized(windowHandle, width, height);
			}
			OnCanvasResized();

			new ResizeObserver(OnCanvasResized).observe(canvas);
		});

	KEEP_IN_MODULE void EMSOnWindowClosed(GLFWwindow* windowHandle)
	{
		glfwSetWindowShouldClose(windowHandle, true);
	}

	EM_JS(void, EMSSetCloseCallback, (),
	{
		const canvas = document.getElementById('canvas');
		window.addEventListener("beforeunload", function(event)
		{
			const windowHandle = canvas.dataset.windowHandle;
			_EMSOnWindowClosed(windowHandle);
		});
	});

	Window::Window(const WindowProps& props)
		: m_Width(props.width), m_Height(props.height)
	{
		if (!glfwInit())
		{
			// TODO: Using library logger and assertion instead
			std::cerr << "Could not init GLFW\n";
			assert(false && "Could not init GLFW");
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		m_WindowHandle = glfwCreateWindow(props.width, props.height, props.title.c_str(), nullptr, nullptr);
		if (!m_WindowHandle)
		{
			glfwTerminate();

			std::cerr << "Could not open GLFW window\n";
			assert(false && "Could not open GLFW window");
		}

		glfwSetWindowUserPointer(m_WindowHandle, this);

		// Handle events
		m_Events.reserve(8); // Hold 8 events without resizing the vector, for now

#if defined(PLATFORM_WEB)
		EMSSetWindowHandle(m_WindowHandle);
		EMSSetWindowTitle(props.title.c_str());
		EMSSetResizeCallback();
		EMSSetCloseCallback();
#endif
		glfwSetWindowCloseCallback(m_WindowHandle, OnWindowClosed);
		glfwSetWindowSizeCallback(m_WindowHandle, OnWindowResized);

		glfwSetKeyCallback(m_WindowHandle, OnKeyPressed);
		glfwSetMouseButtonCallback(m_WindowHandle, OnMousePressed);
		glfwSetScrollCallback(m_WindowHandle, OnMouseScrolled);
		glfwSetCursorPosCallback(m_WindowHandle, OnMouseMoved);

		/*/ Graphics Context */
		// Needs to be initialized after EMSSetResizeCallback,
		// because we need the correct size of the target window (canvas)
		GraphicsContext::Init(m_WindowHandle, m_Width, m_Height);
	}

	void Window::Update()
	{
		m_Events.clear();
		glfwPollEvents();
	}

	Window::~Window()
	{
		glfwDestroyWindow(m_WindowHandle);
		glfwTerminate();
	}

	void Window::OnWindowClosed(GLFWwindow* windowHandle)
	{
		Window* window = static_cast<Window*>(glfwGetWindowUserPointer(windowHandle));
		window->m_Events.push_back(Event(WindowClosedEvent{}));
	}

	void Window::OnWindowResized(GLFWwindow* windowHandle, int width, int height)
	{
		Window* window = static_cast<Window*>(glfwGetWindowUserPointer(windowHandle));
		window->m_Width = static_cast<uint32_t>(width);
		window->m_Height = static_cast<uint32_t>(height);
		window->m_Events.push_back(Event(WindowResizedEvent{ .width = static_cast<uint32_t>(width), .height = static_cast<uint32_t>(height) }));
	}

	void Window::OnKeyPressed(GLFWwindow* windowHandle, int key, int scancode, int action, int mods)
	{
		Window* window = static_cast<Window*>(glfwGetWindowUserPointer(windowHandle));
		if (action == GLFW_RELEASE)
		{
			window->m_Events.push_back(Event(KeyReleasedEvent{ .key = key }));
		}
		else
		{
			window->m_Events.push_back(Event(KeyPressedEvent{ .key = key, .repeat = action == GLFW_PRESS }));
		}
	}

	void Window::OnMousePressed(GLFWwindow* windowHandle, int button, int action, int mods)
	{
		Window* window = static_cast<Window*>(glfwGetWindowUserPointer(windowHandle));

		if (action == GLFW_RELEASE)
		{
			window->m_Events.push_back(Event(MouseReleasedEvent{ .button = button }));
		}
		else
		{
			window->m_Events.push_back(Event(MousePressedEvent{ .button = button, .repeat = action == GLFW_PRESS }));
		}
	}

	void Window::OnMouseScrolled(GLFWwindow* windowHandle, double xoffset, double yoffset)
	{
		Window* window = static_cast<Window*>(glfwGetWindowUserPointer(windowHandle));
		window->m_Events.push_back(Event(MouseScrolledEvent{ .xOffset = static_cast<float>(xoffset), .yOffset = static_cast<float>(yoffset) }));
	}

	void Window::OnMouseMoved(GLFWwindow* windowHandle, double xpos, double ypos)
	{
		Window* window = static_cast<Window*>(glfwGetWindowUserPointer(windowHandle));
		window->m_Events.push_back(Event(MouseMovedEvent{ .xPos = static_cast<float>(xpos), .yPos = static_cast<float>(ypos) }));
	}

}
