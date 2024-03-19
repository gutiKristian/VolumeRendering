#pragma once

#include <array>
#include <vector>
#include <string>
#include <filesystem>

namespace med
{
	/**
	 * @brief Parses a string of numbers separated by '\\' into an array of doubles.
	 *
	 * The function expects the string to be in the format "1\\0\\0\\0\\1\\0". It also handles optional square brackets at the beginning and end of the string.
	 * If the string contains more numbers than the size of the array, the excess numbers are ignored. If it contains less, the remaining elements of the array are set to 0.0.
	 * If a number cannot be parsed, it is skipped and does not affect the array.
	 *
	 * @tparam N The size of the array to be returned, when N = 0 no limit is set and the array will contain all parsed numbers.
	 * @param str The string to be parsed. This parameter is passed by reference and will be modified by the function.
	 * @return An array of doubles parsed from the string.
	 */
	template <typename T, size_t N>
	[[nodiscard]] static std::array<T, N> ParseStringToNumArr(const std::string& str);

	/**
	* @brief Parses a string of numbers separated by '\\' into a vector of floats. Difference to ParseStringToNumArr is that func works with heap memory.
	* @param str The string to be parsed.
	* @return A vector of floats parsed from the string.
	*/
	[[nodiscard]] static std::vector<float> ParseContours(const std::string& str);
}

#include "DicomParseUtil.inl"