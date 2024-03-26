#include "StructureFileDcm.h"
#include "Base/Base.h"

#include <string>
#include <cassert>
#include <limits>

namespace med
{
	StructureFileDcm::StructureFileDcm(std::filesystem::path path, DicomStructParams params, std::vector<std::vector<std::vector<float>>> data) : 
		m_Path(path), m_Params(params), m_Data(data)
	{
	}

	DicomBaseParams med::StructureFileDcm::GetBaseParams() const
	{
		return m_Params;
	}

	DicomModality StructureFileDcm::GetModality() const
	{
		return m_Params.Modality;
	}
	
	bool med::StructureFileDcm::CompareFrameOfReference(const IDicomFile& other) const
	{
		return m_Params.FrameOfReference == other.GetBaseParams().FrameOfReference;
	}

	std::shared_ptr<VolumeFileDcm> StructureFileDcm::Create3DMask(const IDicomFile& other, std::array<int, 4> contourIDs, ContourPostProcess postProcessOpt) const
	{
		if ((postProcessOpt & ContourPostProcess::IGNORE) && (~1 & postProcessOpt))
		{
			LOG_WARN("Ambiguous input for contour post process. Post processing will be ignored");
			postProcessOpt = ContourPostProcess::IGNORE;
		}

		// Assuming same orientation
		if (other.GetModality() != DicomModality::CT || !CompareFrameOfReference(other))
		{
			LOG_CRITICAL("3D mask cannot be created, refernece modality is not CT or Frame of reference does not match with contour's");
			return nullptr;
		}
		
		std::vector<int> finalContours{};
		for (auto id : contourIDs)
		{
			if (id < 0 || id >= m_Data.size())
			{
				std::string str = "Contour ID [ " + std::to_string(id) + "] out of bounds, available [" + std::to_string(m_Data.size()) + "]";
				LOG_WARN(str.c_str());
			}
			else if (id == 0)
			{
				LOG_WARN("Contour ID [0], ignoring. Note, contours are numbered from 1");
			}
			else
			{
				// Existing contours
				finalContours.push_back(id - 1);
			}
		}

		// cast reference to VolumeFileDcm
		const VolumeFileDcm& reference = dynamic_cast<const VolumeFileDcm&>(other);

		auto[xSize, ySize, zSize] = reference.GetSize();
		std::vector<glm::vec4> maskData(xSize * ySize * zSize, { 0.0f, 0.0f, 0.0f, 0.0f });

		// uint8_t is enough for mask data but beacuse of VolumeFile implementation we need to use glm::vec4 for now!
		//std::vector<std::array<uint8_t, 4>> maskData(xSize * ySize * zSize, { 0, 0, 0, 0 });

		// We assume that the ImagePositionPatient is stored from the first slice
		auto[ox, oy, oz] = reference.GetVolumeParams().ImagePositionPatient;
		auto[sx, sy] = reference.GetVolumeParams().PixelSpacing;
		auto sz = reference.GetVolumeParams().SliceThickness;

		glm::vec3 spacing{ sx, sy, sz };
		glm::vec3 origin{ ox, oy, oz };

		// Traverse contours
		for (size_t l = 0; l < finalContours.size(); ++l)
		{
			const auto cId = finalContours[l];
			// Pick contour
			for (size_t i = 0; i < m_Data[cId].size(); ++i)
			{
				assert(m_Data[cId][i].size() % 3 == 0);
				// Traverses images where contours are defined
				for (size_t j = 0; j <= m_Data[cId][i].size() - 3; j+=3)
				{
					// RCS -> Voxel
					glm::vec3 contourPoint{ m_Data[cId][i][j], m_Data[cId][i][j + 1], m_Data[cId][i][j + 2] };
					glm::vec3 voxel = (contourPoint - origin) / spacing;
					voxel.z = std::fabs(voxel.z);
					voxel = glm::round(voxel);

					// 3D coordinates -> 1D coordinate; reference is the same size as the mask
					int index = reference.GetIndexFrom3D(static_cast<int>(voxel.x), static_cast<int>(voxel.y), static_cast<int>(voxel.z));
					
					if (index == -1)
					{
						LOG_WARN("Contour point out of bounds, skipping...");
						continue;
					}

					if ((maskData[index][l] == 1 || postProcessOpt & PROCESS_NON_DUPLICATES) && !(postProcessOpt & ContourPostProcess::IGNORE))
					{
						if (postProcessOpt & ContourPostProcess::NEAREST_NEIGHBOUR)
						{
							glm::ivec2 substituteVoxel = HandleDuplicatesNearestNeighbour(reference, contourPoint, voxel);
							int newIndex = reference.GetIndexFrom3D(substituteVoxel.x, substituteVoxel.y, static_cast<int>(voxel.z));
							maskData[newIndex][l] = 1;
						}

						if (postProcessOpt & ContourPostProcess::RECONSTRUCT_BRESENHAM)
						{
							if (j + 3 <= m_Data[cId][i].size() - 3)
							{
								glm::vec3 contourNext{ m_Data[cId][i][j + 3], m_Data[cId][i][j + 4], m_Data[cId][i][j + 5] };
								glm::vec3 nextVoxel = glm::round((contourNext - origin) / spacing);
								auto derivedVoxels = HandleDuplicatesLineToNextBresenahm(reference, voxel, nextVoxel);
								assert(derivedVoxels.back().x == nextVoxel.x && derivedVoxels.back().y == nextVoxel.y && "Bresenham issue");

								// We must exclude last derived voxel, this is next voxel and we don't want it to be flagged as duplicate, next iter.
								for (size_t i = 0; i < derivedVoxels.size() - 1; ++i)
								{
									index = reference.GetIndexFrom3D(derivedVoxels[i].x, derivedVoxels[i].y, static_cast<int>(voxel.z));
									maskData[index][l] = 1;
								}
							}
						}

					}
					// Activate point at index, l is the contour we are processing
					maskData[index][l] = 1;
				}
			}
		}

		return std::make_shared<VolumeFileDcm>(m_Path, reference.GetSize(), FileDataType::Float, reference.GetVolumeParams(), maskData);
	}

	glm::ivec2 StructureFileDcm::HandleDuplicatesNearestNeighbour(const VolumeFileDcm& reference, glm::vec3 currentRCS, glm::vec3 currentVoxel) const
	{
		auto [xSize, ySize, zSize] = reference.GetSize();
		float minDist = std::numeric_limits<float>::max();
		glm::ivec2 substituteVoxel(currentVoxel.x, currentVoxel.y);

		// Since contours are on slices here we work with images
		for (int i = -1; i <= 1; ++i)
		{
			for (int j = -1; j <= 1; ++j)
			{
				if (i == 0 && j == 0)
				{
					continue;
				}

				int newX = currentVoxel.x + j;
				int newY = currentVoxel.y + i;

				// I could check also the 3D coord and use more neighbours, but I suppose the contours are
				// created on the slices so the error in z is not there
				if (newX >= 0 && newX < xSize && newY >= 0 && newY <= ySize)
				{
					glm::vec2 newVoxel(newX, newY);
					glm::vec3 newRCS = reference.PixelToRCSTransform(newVoxel);
					float currentDistance = glm::distance(currentRCS, newRCS);
					if (currentDistance < minDist)
					{
						minDist = currentDistance;
						substituteVoxel.x = newX;
						substituteVoxel.y = newY;
					}
				}
			}
		}

		return substituteVoxel;
	}

	std::vector<glm::ivec2> StructureFileDcm::HandleDuplicatesLineToNextBresenahm(const const VolumeFileDcm& reference, glm::vec3 start, glm::vec3 end) const
	{
		std::vector<glm::ivec2> res{};
		int dX = end.x - start.x;
		int dY = end.y - start.y;

		int step = 0;

		int currentX = start.x;
		int currentY = start.y;

		int incrX = dX > 0 ? 1 : -1;
		int incrY = dY > 0 ? 1 : -1;

		int dValue = -std::abs(dX);

		// how many steps we are going to make
		if (std::abs(dX) > std::abs(dY))
		{
			step = std::abs(dX);
		}
		else
		{
			step = std::abs(dY);
			dValue = -std::abs(dY);
		}

		for (int i = 0; i <= step; ++i)
		{
			res.emplace_back(currentX, currentY);

			if (std::abs(dX) > std::abs(dY))
			{
				// sampling x axis
				dValue = dValue + (2 * std::abs(dY));
				if (dValue >= 0)
				{
					currentX += incrX;
					currentY += incrY;
					dValue = dValue - (2 * std::abs(dX));
				}
				else
				{
					currentX += incrX;
				}
			}
			else
			{
				dValue = dValue + (2 * std::abs(dX));
				if (dValue >= 0)
				{
					currentX += incrX;
					currentY += incrY;
					dValue = dValue - (2 * std::abs(dY));
				}
				else
				{
					currentY += incrY;
				}
			}
		}

		return res;
	}
}