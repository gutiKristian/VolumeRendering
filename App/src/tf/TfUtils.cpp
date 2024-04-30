#include "TfUtils.h"
#include "Base/Base.h"

#include <algorithm>
#include <cassert>

namespace med
{
	void TfUtils::CheckDragBounds(int index, std::vector<glm::dvec2>& controlPoints, int maxDataVal)
	{
		const double X_MAX = maxDataVal - 1.0; // Depends on input data depth
		const double X_MIN = 0.0;

		// We must use floor in order to have at least one unit padding between two cps
		auto& cp = controlPoints[index];
		const auto cpSize = controlPoints.size();

		// Clamp the control point to the plot area
		cp.y = std::clamp(cp.y, 0.0, 1.0);
		cp.x = std::clamp(cp.x, X_MIN, X_MAX);

		// Is bounded from left
		if (index - 1 >= 0 && controlPoints[index - 1].x >= (cp.x - 1.0))
		{
			cp.x = std::ceil(controlPoints[index - 1].x + 1.0);
		}
		// Is bounded from right
		if (index + 1 < cpSize && controlPoints[index + 1].x <= (cp.x + 1.0))
		{
			cp.x = std::floor(controlPoints[index + 1].x - 1.0);
		}

	}

	std::unique_ptr<OpacityTF> TfUtils::CalibrateOpacityTF(std::shared_ptr<const VolumeFile> mask, std::shared_ptr<const VolumeFile> file, std::array<int, 4> activeContours, int tfResolution)
	{
		// Checking
		if (file == nullptr || mask == nullptr)
		{
			LOG_ERROR("TF Calibration: NULLPTR, TF won't be calibrated");
			return std::make_unique<OpacityTF>(tfResolution);
		}

		auto [x, y, z] = mask->GetSize();
		auto [xx, yy, zz] = file->GetSize();

		// Matching size check
		if (x != xx || y != yy || z != zz)
		{
			LOG_ERROR("TF calibration, mask and data sizes do not match, TF won't be calibrated");
			return std::make_unique<OpacityTF>(tfResolution);
		}
		
		size_t size = x * y * z;

		// Empty file check
		if (size == 0)
		{
			LOG_ERROR("TF calibration: empty file, TF won't be calibrated!");
			return std::make_unique<OpacityTF>(tfResolution);
		}


		size_t maxValue = file->GetMaxNumber();

		const auto& maskData = mask->GetVecReference();
		const auto& fileData = file->GetVecReference();

		assert(maskData.size() == fileData.size() && "Underlying data are not the same size");

		// Max value check
		if (maxValue == 0)
		{
			LOG_WARN("Max value is zero, looking for max value.");
			maxValue = file->GetMaxNumber(fileData, 3); // alpha --> rgb may contain precomputed gradient
			if (maxValue == 0)
			{
				LOG_ERROR("After check: max value is 0, TF won't be calibrated.");
				return std::make_unique<OpacityTF>(tfResolution);
			}
		}

		std::vector<int> cIndices{};
		for (int i = 0; i < activeContours.size(); ++i)
		{
			if (activeContours[i] == 1)
			{
				cIndices.push_back(i);
			}
		}

		// Check whether user flagged at least one contour
		if (cIndices.size() == 0)
		{
			LOG_ERROR("No contour has been selected for calibration, TF won't be calibrated!");\
			return std::make_unique<OpacityTF>(tfResolution);
		}

		// calculate histogram inside the contour
		std::vector<int> bin(maxValue, 0);

		for (size_t i = 0; i < size; ++i)
		{
			for (auto contourIndex : cIndices)
			{
				if (maskData[i][contourIndex] != 0)
				{
					// Active contour
					int value = fileData[i].a;
					++bin[value];
				}
			}
		}
		
		return std::make_unique<OpacityTF>(tfResolution);
	}
}