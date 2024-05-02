#pragma once
#include <glm/glm.hpp>

namespace med
{

	// Directional, although we need only rgb, padding is added for alignment -> wgpu aligns to 16 bytes
	// no spec
	struct Light
	{
		glm::vec4 Position{ 0.0f };
		glm::vec4 Ambient{ 0.0f };
		glm::vec4 Diffuse{ 0.0f };
	};
}