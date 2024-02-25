#include <vector>
#include <concepts>
#include "glm/glm.hpp"

namespace med
{
	class LinearInterpolation
	{
	public:
		template<typename T, typename U>
		static std::vector<T> Generate(U x0, U x1, T fx0, T fx1, U step)
		{
			static_assert(std::is_arithmetic<U>::value);
			std::vector<T> result{};

			auto slope = (fx1 - fx0) * (1.0f / static_cast<float>(x1 - x0));

			for (auto i = x0; i < x1 + 1; i += step)
			{
				if constexpr (std::is_same<T, glm::vec3>::value)
				{
					float r = fx0.r + slope.r * (i - x0);
					float g = fx0.g + slope.g * (i - x0);
					float b = fx0.b + slope.b * (i - x0);
					result.emplace_back(r, g, b);
				}
				else
				{
					result.push_back(fx0 + slope * (i - x0));
				}
			}

			return result;
		}
	};
}