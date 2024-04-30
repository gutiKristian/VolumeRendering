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
		
		/*
		* @brief Calibrates transfer function based on the provided mask. The mask is usually a filled contour.
		* @param mask: The mask which determines the areas where calibration is performed
		* @param data: File used to generate the histogram inside the areas
		* @param activeContours: As mask may have up to 4 contours, this param tells us which should be taken into account
		* @param tfResolution: Resolution of the texture
		* @return Calibrated TF, if some of the checks fail default OpacityTF is returned
		*/
		static std::unique_ptr<OpacityTF> CalibrateOpacityTF(std::shared_ptr<const VolumeFile> mask, std::shared_ptr<const VolumeFile> data, std::array<int, 4> activeContours, int tfResolution);
	};
}