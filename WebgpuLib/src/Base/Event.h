#pragma once

#include <cstdint>

namespace base {

	enum class EventType
	{
		WindowClosed = 0, WindowResized,
		KeyPressed, KeyReleased, KeyTyped,
		MousePressed, MouseReleased, MouseMoved, MouseScrolled
	};

	struct WindowClosedEvent
	{
	};

	struct WindowResizedEvent
	{
		uint32_t width, height;
	};

	struct KeyPressedEvent
	{
		int32_t key;
		bool repeat;
	};

	struct KeyReleasedEvent
	{
		int32_t key;
	};

	struct MousePressedEvent
	{
		int32_t button;
		bool repeat;
	};

	struct MouseReleasedEvent
	{
		int32_t button;
	};

	struct MouseScrolledEvent
	{
		float xOffset, yOffset;
	};

	struct MouseMovedEvent
	{
		float xPos, yPos;
	};

	struct Event
	{
		const EventType type;
		const union
		{
			WindowClosedEvent windowClosedEvent;
			WindowResizedEvent windowResizedEvent;

			KeyPressedEvent keyPressedEvent;
			KeyReleasedEvent keyReleasedEvent;
			MousePressedEvent mousePressedEvent;
			MouseReleasedEvent mouseReleasedEvent;

			MouseScrolledEvent mouseScrolledEvent;
			MouseMovedEvent mouseMovedEvent;
		} as;

		Event(const WindowClosedEvent& ev)
			: type(EventType::WindowClosed), as({ .windowClosedEvent = ev }) {}

		Event(const WindowResizedEvent& ev)
			: type(EventType::WindowResized), as({ .windowResizedEvent = ev }) {}

		Event(const KeyPressedEvent& ev)
			: type(EventType::KeyPressed), as({ .keyPressedEvent = ev }) {}

		Event(const KeyReleasedEvent& ev)
			: type(EventType::KeyReleased), as({ .keyReleasedEvent = ev }) {}

		Event(const MousePressedEvent& ev)
			: type(EventType::MousePressed), as({ .mousePressedEvent = ev }) {}

		Event(const MouseReleasedEvent& ev)
			: type(EventType::MouseReleased), as({ .mouseReleasedEvent = ev }) {}

		Event(const MouseScrolledEvent& ev)
			: type(EventType::MouseScrolled), as({ .mouseScrolledEvent = ev }) {}

		Event(const MouseMovedEvent& ev)
			: type(EventType::MouseMoved), as({ .mouseMovedEvent = ev }) {}
	};

}
