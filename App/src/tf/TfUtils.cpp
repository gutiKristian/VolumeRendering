#include "TfUtils.h"
#include "Base/Base.h"

#include <algorithm>

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
}