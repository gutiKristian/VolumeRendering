#pragma once

#include "Base/Event.h"

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

struct GLFWwindow;

namespace base {

	struct WindowProps
	{
		uint32_t width, height;
		std::string title;
	};

	class Window
	{
	public:
		Window(const WindowProps& props);
		~Window();

		void Update();

		const std::vector<Event>& GetEvents() const { return m_Events; }
		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }
		std::pair<uint32_t, uint32_t> GetWindowSize() const { return std::make_pair(m_Width, m_Height); }
		GLFWwindow* GetWindowHandle() const { return m_WindowHandle; }
	private:
		static void OnWindowClosed(GLFWwindow* windowHandle);
		static void OnWindowResized(GLFWwindow* windowHandle, int width, int height);

		static void OnKeyPressed(GLFWwindow* windowHandle, int key, int scancode, int action, int mods);
		static void OnMousePressed(GLFWwindow* windowHandle, int button, int action, int mods);
		static void OnMouseScrolled(GLFWwindow* windowHandle, double xoffset, double yoffset);
		static void OnMouseMoved(GLFWwindow* windowHandle, double xpos, double ypos);
	private:
		uint32_t m_Width, m_Height;
		GLFWwindow* m_WindowHandle = nullptr;

		std::vector<Event> m_Events;
	};

}
