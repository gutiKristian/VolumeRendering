#include <vector>
#include <concepts>

namespace med
{
	class LinearInterpolation
	{
	public:
		template<typename T>
		static std::vector<T> Generate(auto x0, auto x1, auto fx0, auto fx1, auto step)
		{
            static_assert(std::is_arithmetic<T>::value);
            std::vector<T> result{};

			auto c1 = (fx1 - fx0) / (x1 - x0);

			for (auto i = x0; i < x1+1; i+=step)
			{
				result.push_back(fx0 + c1 * (i - x0));
			}

			return result;
		}
	};
}