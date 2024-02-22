#pragma once

#include <array>

namespace med
{
	struct DicomParams
	{
		std::uint16_t X = 0, Y = 0, Z = 0, BitsStored = 0, BitsAllocated = 0; // US
		std::int16_t NumberOfFrames = 0; // IS
		double SliceThickness = 0.0; // DS
		std::array<double, 3> ImagePositionPatient{ 0.0 };
		std::array<double, 3> ImageOrientationPatient{ 0.0 };
		std::array<double, 2> PixelSpacing{ 0.0 };
	};
}