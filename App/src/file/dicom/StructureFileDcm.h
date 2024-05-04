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
		FILL = 1 << 4,

		PROCESS_NON_DUPLICATES = 1 << 5
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

		DicomStructParams GetStructParams() const;

		void ListAvailableContours() const;

		/*
		* @brief Creates a volume file (volume mask) from the contour data.
		* @param other: Reference dicom file, file from which this contours were generated
		* @param contourIDs: IDs of contours to be included inside the max, up to 4 contours can be selected
		* @param postProcess: What post process actions should be performed on the data.
		*/
		std::shared_ptr<VolumeFileDcm> Create3DMask(const IDicomFile& other, std::array<int, 4> contourIDs, ContourPostProcess duplicateOption);
		
		/* IDicom Interface */
		DicomBaseParams GetBaseParams() const override;

		DicomModality GetModality() const override;

		bool CompareFrameOfReference(const IDicomFile& other) const override;
	private:		
		/*
		* @brief Given point in the RCS and its voxel space mapping we find nearest point to this concrete point.
		* @param reference: Reference dataset, dataset w.r.t we calculate the mask
		* @param currentRCS: Point's coordinate in RCS (Reference coordinate system), used when measuring distance
		* @param currentVoxel: Pont's voxel space coordinate, used when deriving neighbours
		* @return: Coordinates of nearest neighbour in voxel space
		*/
		glm::ivec2 HandleDuplicatesNearestNeighbour(const VolumeFileDcm& reference, glm::vec3 currentRCS, glm::vec3 currentVoxel);
		/*
		* @brief Draws Line from start to end using Bresenham line drawing algorithm
		* @param reference: Reference dataset, dataset w.r.t we calculate the mask
		* @param start: Starting point of the line
		* @param end: Ending point (not included in the result to avoid marking it as duplicate)
		* @return: Vector of voxel space coordinates, these coordinates are within @reference's voxel space
		*/
		std::vector<glm::ivec2> HandleDuplicatesLineToNextBresenahm(const const VolumeFileDcm& reference, glm::vec3 start, glm::vec3 end);

		/*
		* @brief Perform Dilation or Erosion on passed data (these data are changed!)
		* @param xSize Width of the slice/image within the 3D mask
		* @param ySize Height of the slice/image within the 3D mask
		* @param sliceNumber Number of the image that is being processed in the 3D mask
		* @param structureElement Structuring element used in the morph. op.
		* @param doErosion If true Erosion is performed, dilation otherwise
		* @return None, data are changed inside the method
		*/
		void MorphologicalOp(std::vector<glm::vec4>& data, int xSize, int ySize, int sliceNumber,
			std::vector<std::vector<uint8_t>> structureElement, bool doErosion);

		glm::vec2 FindSeed(int yStart, int xSize, int ySize, int sliceNumber, int contourNumber, std::vector<glm::vec4>& data);
		void FloodFill(glm::ivec2 seed, int xSize, int ySize, int sliceNumber, int contourNumber, std::vector<glm::vec4>& data);


	private:
		std::filesystem::path m_Path;
		DicomStructParams m_Params;
		std::vector<std::vector<std::vector<float>>> m_Data;
		std::vector<int> m_ActiveContourIDs{};
	};
}