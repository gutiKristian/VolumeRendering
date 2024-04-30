#include <vector>
#include <glm/glm.hpp>

namespace med
{
	class TfUtils
	{
	public:

		static void CheckDragBounds(int index, std::vector<glm::dvec2>& controlPoints, int maxDataVal);
	};
}