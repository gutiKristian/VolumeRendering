#include "TfUtils.h"
#include "Base/Base.h"

#include <algorithm>

namespace med
{
	int TfUtils::AddControlPoint(double x, double y, std::vector<glm::dvec2>& cp)
	{
		constexpr int CONTROL_POINT_EXISTS = -1;
		auto findPoint = [](const auto& obj, double x) { return obj.x < x; };

		// On x-axis we always work with nearest integer values -- from 0 to 'bit depth of the voxel data'
		x = std::round(x);

		// Retrieve the index of this point if already exist
		auto iter = std::lower_bound(cp.begin(), cp.end(), x, findPoint);

		if (iter == cp.end() || x == (*iter).x)
		{
			LOG_TRACE("Control point already exists, drag it to edit its value");
			return CONTROL_POINT_EXISTS;
		}

		// Create Implot handles for control points
		cp.emplace_back(x, y);

		// Helps us recalculate data between the neighbourhood points
		std::ranges::sort(cp, [](const auto& a, const auto& b) {return a.x < b.x; });

		iter = std::lower_bound(cp.begin(), cp.end(), x, findPoint);

		LOG_TRACE("Added new point");
		return std::distance(cp.begin(), iter);
	}

	void TfUtils::CheckDragBounds(size_t index, std::vector<glm::dvec2>& controlPoints, int maxDataVal)
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
}