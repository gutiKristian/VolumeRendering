#pragma once

#include "IDicomFile.h"
#include "DicomParams.h"
#include "VolumeFileDcm.h"

#include <vector>
#include <memory>
#include <array>
#include <filesystem>

namespace med
{
	enum ContourPostProcess : unsigned int
	{
		IGNORE = 1 << 0,
		NEAREST_NEIGHBOUR = 1 << 1,
		RECONSTRUCT_BRESENHAM = 1 << 2,
		CLOSING = 1 << 3,

		PROCESS_NON_DUPLICATES = 1 << 4
	};

	inline ContourPostProcess operator|(ContourPostProcess a, ContourPostProcess b)
	{
		return static_cast<ContourPostProcess>(static_cast<int>(a) | static_cast<int>(b));
	}

	class StructureFileDcm : public IDicomFile
	{
	public:
		StructureFileDcm(std::filesystem::path path, DicomStructParams params, std::vector<std::vector<std::vector<float>>> data);
		
	public:
		DicomBaseParams GetBaseParams() const override;
		DicomModality GetModality() const override;
		bool CompareFrameOfReference(const IDicomFile& other) const override;
		std::shared_ptr<VolumeFileDcm> Create3DMask(const IDicomFile& other, std::array<int, 4> contourIDs, ContourPostProcess duplicateOption) const;
	private:
		glm::ivec2 HandleDuplicatesNearestNeighbour(const VolumeFileDcm& reference, glm::vec3 currentRCS, glm::vec3 currentVoxel) const;
		std::vector<glm::ivec2> HandleDuplicatesLineToNextBresenahm(const const VolumeFileDcm& reference, glm::vec3 start, glm::vec3 end) const;
	private:
		std::filesystem::path m_Path;
		DicomStructParams m_Params;
		std::vector<std::vector<std::vector<float>>> m_Data;
	};
}