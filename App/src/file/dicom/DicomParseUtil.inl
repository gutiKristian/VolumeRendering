#pragma once

#include <array>
#include <vector>
#include <string>
#include <sstream>
#include <limits>

#include "Base/Log.h"
#include "DicomParseUtil.h"


namespace med
{
	template <typename T, size_t N>
	[[nodiscard]] static std::array<T, N> ParseStringToNumArr(const std::string& str)
	{
		// the string is in this format "1\\0\\0\\0\\1\\0"
		std::array<T, N> numbers{ 0.0 };

		// then parse the string expected number of numbers is specified by N
		std::stringstream ss(str);
		std::string temp{};
		size_t i = 0;
		while (std::getline(ss, temp, '\\') && (N == 0 || i < N))
		{
			try
			{
				if constexpr (std::is_same_v<T, int>)
				{
					numbers[i++] = std::stoi(temp);
				}
				else if constexpr (std::is_same_v<T, std::uint16_t>)
				{
					int number = std::stoi(temp);
					if (number < 0)
					{
						LOG_WARN("Casting negative number to unsigned, possible data loss");
						number = 0;
					}
					if (number > std::numeric_limits<std::uint16_t>::max())
					{
						LOG_WARN("Casting to uin16 overflow, possible data loss");
						number = std::numeric_limits<std::uint16_t>::max();
					}
					numbers[i++] = static_cast<std::uint16_t>(number);
				}
				else if constexpr (std::is_same_v<T, std::uint32_t>)
				{
					numbers[i++] = static_cast<std::uint32_t>(std::stoul(temp));
				}
				else if constexpr (std::is_same_v<T, double>)
				{
					numbers[i++] = std::stod(temp);
				}
				else if constexpr (std::is_same_v<T, float>)
				{
					numbers[i++] = std::stof(temp);
				}
				else
				{
					LOG_ERROR("Unsupported type");
				}
			}
			catch (std::exception& e)
			{
				// do nothing, just skip the number (decide in the future)
			}
		}
		return numbers;
	};

	[[nodiscard]] static std::vector<float> ParseContours(const std::string& str)
	{
		std::vector<float> res{};
		std::stringstream ss(str);
		std::string temp;
		while (std::getline(ss, temp, '\\'))
		{
			try
			{
				res.push_back(std::stof(temp));
			}
			catch (std::exception& e)
			{
				LOG_ERROR("Parsing contour data failed!");
			}
		}
		return res;
	}
}