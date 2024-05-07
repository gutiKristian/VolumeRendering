#pragma once

#include "OpacityTf.h"
#include "../file/VolumeFile.h"

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <array>

namespace med
{
	class TfUtils
	{
	public:
		static void CheckDragBounds(int index, std::vector<glm::dvec2>& controlPoints, int maxDataVal);
		
	};
}