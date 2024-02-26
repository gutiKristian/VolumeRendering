#include <vector>
#include <glm/glm.hpp>

namespace med
{
	class TfUtils
	{
	public:
		/*
		* @brief Add a control point to the control points vector.
		* If new control point is added, sorts the control points vector by x coordinate.
		* @return The index of the added control point or -1 if already exists.
		*/
		static int AddControlPoint(double x, double y, std::vector<glm::dvec2>& cp);

		static void CheckDragBounds(int index, std::vector<glm::dvec2>& controlPoints, int maxDataVal);
	};
}