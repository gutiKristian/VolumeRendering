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

	std::shared_ptr<VolumeFileDcm> StructureFileDcm::Create3DMask(const IDicomFile& other, std::array<int, 4> contourIDs, bool handleDuplicates) const
	{
		// Assuming same orientation
		if (other.GetModality() != DicomModality::CT || !CompareFrameOfReference(other))
		{
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
					glm::vec3 contourPoint{ m_Data[cId][i][j], m_Data[cId][i][j + 1], m_Data[cId][i][j + 2] };
					glm::vec3 voxel = (contourPoint - origin) / spacing;
					voxel.z = std::fabs(voxel.z);
					voxel = glm::round(voxel);

					// reference is the same size as the mask
					int index = reference.GetIndexFrom3D(static_cast<int>(voxel.x), static_cast<int>(voxel.y), static_cast<int>(voxel.z));
					
					if (index == -1)
					{
						LOG_WARN("Contour point out of bounds, skipping...");
					}
					else
					{
						if (maskData[index][l] == 1 && handleDuplicates)
						{
							float minDist = std::numeric_limits<float>::max();
							glm::ivec2 substituteVoxel(0, 0);

							// Since contours are on slices here we work with images
							for (int i = -1; i <= 1; ++i)
							{
								for (int j = -1; j <= 1; ++j)
								{
									if (i == 0 && j == 0)
									{
										continue;
									}

									int newX = voxel.x + j;
									int newY = voxel.y + i;
									
									// I could check also the 3D coord and use more neighbours, but I suppose the contours are
									// created on the slices so the error in z is not there
									if (newX >= 0 && newX < xSize && newY >= 0 && newY <= ySize)
									{
										glm::vec2 current(newX, newY);
										glm::vec3 rcs = reference.PixelToRCSTransform(current);
										float currentDistance = glm::distance(contourPoint, rcs);
										if (currentDistance < minDist)
										{
											minDist = currentDistance;
											substituteVoxel.x = newX;
											substituteVoxel.y = newY;
										}
									}
								}
							}

							index = reference.GetIndexFrom3D(static_cast<int>(substituteVoxel.x), static_cast<int>(substituteVoxel.y), static_cast<int>(voxel.z));
						}

						maskData[index][l] = 1;
					}
				}
			}
		}

		return std::make_shared<VolumeFileDcm>(m_Path, reference.GetSize(), FileDataType::Float, reference.GetVolumeParams(), maskData);
	}
}