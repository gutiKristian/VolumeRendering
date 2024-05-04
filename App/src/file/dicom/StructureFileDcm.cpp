#include "StructureFileDcm.h"
#include "Base/Base.h"

#include <glm/glm.hpp>
#include <string>
#include <sstream>
#include <cassert>
#include <limits>
#include <queue>

namespace med
{
	StructureFileDcm::StructureFileDcm(std::filesystem::path path, DicomStructParams params, std::vector<std::vector<std::vector<float>>> data) :
		m_Path(path), m_Params(params), m_Data(data)
	{
	}

	DicomStructParams StructureFileDcm::GetStructParams() const
	{
		return m_Params;
	}

	void StructureFileDcm::ListAvailableContours() const
	{
		std::stringstream s{};
		s << "Contours:\n";
		for (const auto& elem : GetStructParams().StructureSetROISequence)
		{
			s << "\t" << elem.Number << " : " << elem.Name << elem.AlgorithmType << "\n";
		}
		LOG_INFO(s.str().c_str());
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

	std::shared_ptr<VolumeFileDcm> StructureFileDcm::Create3DMask(const IDicomFile& other, std::array<int, 4> contourIDs, ContourPostProcess postProcessOpt)
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

		std::vector<std::vector<int>> sliceNumbers{};

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
				sliceNumbers.push_back({});
			}
		}

		m_ActiveContourIDs = finalContours;

		// cast reference to VolumeFileDcm
		const VolumeFileDcm& reference = dynamic_cast<const VolumeFileDcm&>(other);

		auto [xSize, ySize, zSize] = reference.GetSize();
		std::vector<glm::vec4> maskData(xSize * ySize * zSize, { 0.0f, 0.0f, 0.0f, 0.0f });

		// uint8_t is enough for mask data but beacuse of VolumeFile implementation we need to use glm::vec4 for now!
		//std::vector<std::array<uint8_t, 4>> maskData(xSize * ySize * zSize, { 0, 0, 0, 0 });

		// We assume that the ImagePositionPatient is stored from the first slice (if the data was divided into multiple files)
		auto [ox, oy, oz] = reference.GetVolumeParams().ImagePositionPatient;
		auto [sx, sy] = reference.GetVolumeParams().PixelSpacing;
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
				int sliceNumber = -1;

				std::vector<float> yCoords{};
				yCoords.reserve(m_Data[cId][i].size());

				// Traverses images where contours are defined
				for (size_t j = 0; j <= m_Data[cId][i].size() - 3; j += 3)
				{
					// RCS -> Voxel
					glm::vec3 contourPoint{ m_Data[cId][i][j], m_Data[cId][i][j + 1], m_Data[cId][i][j + 2] };
					// glm::vec3 voxel = reference.RCSToVoxelTransform(contourPoint); <--- using the matrix and slice thickness
					glm::vec3 voxel = (contourPoint - origin) / spacing;
					voxel.z = std::fabs(voxel.z);
					voxel = glm::round(voxel);
					sliceNumber = static_cast<int>(voxel.z);
					// 3D coordinates -> 1D coordinate; reference is the same size as the mask
					int index = reference.GetIndexFrom3D(static_cast<int>(voxel.x), static_cast<int>(voxel.y), static_cast<int>(voxel.z));

					if (index == -1)
					{
						LOG_WARN("Contour point out of bounds, skipping...");
						continue;
					}

					// if (duplicate or	process non-duplicates anyway) and (ignore flag is deactivated)
					if ((maskData[index][l] == 1 || postProcessOpt & PROCESS_NON_DUPLICATES) && !(postProcessOpt & ContourPostProcess::IGNORE))
					{
						// These post-processes are done on point/pixel level
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
								// add check if out of img
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

					yCoords.push_back(voxel.y);
					// Activate point at index, l is the contour we are processing
					maskData[index][l] = 1;
				}

				// Contour l is defined on slice sliceNumber
				sliceNumbers[l].push_back(sliceNumber);

				// Process the created image
				if (postProcessOpt & ContourPostProcess::CLOSING)
				{
					MorphologicalOp(maskData, xSize, ySize, sliceNumber, { {1,1,1}, {1,1,1}, {1,1,1} }, false); // Dilation
					MorphologicalOp(maskData, xSize, ySize, sliceNumber, { {1,1,1}, {1,1,1}, {1,1,1} }, true); // Erosion
				}

				if (postProcessOpt & ContourPostProcess::FILL)
				{
					std::ranges::sort(yCoords);
					glm::ivec2 seed = FindSeed(static_cast<int>(yCoords[yCoords.size() / 2]), xSize, ySize, sliceNumber, l, maskData);
					if (seed.x == -1)
					{
						LOG_ERROR("Did not find the seed for fill.");
					}
					else
					{
						FloodFill(seed, xSize, ySize, sliceNumber, l, maskData);
					}
				}
			}
		}

		auto file = std::make_shared<VolumeFileDcm>(m_Path, reference.GetSize(), FileDataType::Float, reference.GetVolumeParams(), maskData);
		file->SetContourSliceNumbers(sliceNumbers);
		return file;
	}

	glm::ivec2 StructureFileDcm::HandleDuplicatesNearestNeighbour(const VolumeFileDcm& reference, glm::vec3 currentRCS, glm::vec3 currentVoxel)
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

	std::vector<glm::ivec2> StructureFileDcm::HandleDuplicatesLineToNextBresenahm(const const VolumeFileDcm& reference, glm::vec3 start, glm::vec3 end)
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

	void StructureFileDcm::MorphologicalOp(std::vector<glm::vec4>& data, int xSize, int ySize, int sliceNumber, std::vector<std::vector<uint8_t>> structureElement, bool doErosion)
	{
		if (structureElement.size() == 0 || structureElement[0].size() == 0)
		{
			LOG_ERROR("Structure element is undefined, skipping");
			return;
		}

		if (structureElement.size() % 2 == 0 || structureElement[0].size() % 2 == 0)
		{
			LOG_ERROR("Cannot determine the middle point in structure element, skipping");
			return;
		}

		auto coord3D = [&](int x, int y, int z)
			{
				return z * ySize * xSize + y * xSize + x;
			};

		auto coord2D = [&](int x, int y)
			{
				return y * xSize + x;
			};

		int offy = structureElement.size() / 2;
		int offx = structureElement[0].size() / 2;

		std::vector<glm::vec4>::const_iterator begin = data.begin() + coord3D(0, 0, sliceNumber);
		std::vector<glm::vec4>::const_iterator end = data.begin() + coord3D(0, 0, sliceNumber + 1);
		// Altered stores the slice, where we compute the morphological operation
		std::vector<glm::vec4> altered(begin, end);
		
		assert(altered.size() == xSize * ySize);

		for (int cId = 0; cId < m_ActiveContourIDs.size(); ++cId)
		{
			for (int y = offy; y < ySize - offy; ++y)
			{
				for (int x = offx; x < xSize - offx; x++)
				{
					bool hasMissed = false;
					bool hasHit = false;

					for (int i = -offy; i <= offy; ++i)
					{
						for (int j = -offx; j <= offx; ++j)
						{
							// Point where we attached the structure element or point that is not in structure element
							if (structureElement[i + offy][j + offx] == 0)
								continue;

							int index = coord2D(x + j, y + i);

							if (doErosion && altered[index][cId] == 0)
							{
								hasMissed = true;
								break;
							}
							else if (!doErosion && altered[index][cId] == 1)
							{
								// Dilation
								hasHit = true;
								break;
							}

						}

						if (hasMissed || hasHit)
							break;
					}

					int dataCoord = coord3D(x, y, sliceNumber);

					if (hasMissed)
					{
						data[dataCoord][cId] = 0;
					}
					//else
					//{
					//	// Some struture elements may have non active attachment points
					//	// + active, - disabled, strucure element -> + - + , where point in the middle is attachment point
					//	data[dataCoord][cId] = 1;
					//}

					if (hasHit)
					{
						data[dataCoord][cId] = 1;
					}

				}
			}
		}

	}
	
	glm::vec2 StructureFileDcm::FindSeed(int yStart, int xSize, int ySize, int sliceNumber, int contourNumber, std::vector<glm::vec4>& data)
	{
		int yCurrent = yStart;
		constexpr int MAGIC_THRESH = 5;

		// Make VolumeFile get 3D static -- x, y, z, width, height
		auto GI = [&](int x, int y, int z) -> int
		{
			return z * ySize * xSize + y * xSize + x;
		};

		// Explore only half of the image, we can do this as we start in the middle of the contour
		while (yCurrent >= 0 && yCurrent < ySize)
		{
			for (int x = 0; x < xSize; ++x)
			{
				int i = GI(x, yCurrent, sliceNumber);

				// Default, no contour found
				if (data[i][contourNumber] == 0)
					continue;
				if (data[i][contourNumber] == 1)
				{
					int numOfOnes = 0;
					// shoot ray
					while (x < xSize)
					{
						int ii = GI(x, yCurrent, sliceNumber);
						if (data[ii][contourNumber] == 1)
						{
							++numOfOnes;
						} 
						else if (data[ii][contourNumber] == 0 && numOfOnes < MAGIC_THRESH)
						{
							// We should be inside, magic threshold is here to account for wider edges
							return glm::ivec2(x, yCurrent);
						}
						else if (data[ii][contourNumber] == 0 && numOfOnes >= MAGIC_THRESH)
						{
							// We might have passed already filled contour
							break;
						}
						++x;
					}

				}


			}
			++yCurrent;
		}


		return glm::vec2(-1, -1);
	}

	void StructureFileDcm::FloodFill(glm::ivec2 seed, int xSize, int ySize, int sliceNumber, int contourNumber, std::vector<glm::vec4>& data)
	{
		auto GI = [&](int x, int y, int z) -> int
		{
			return z * ySize * xSize + y * xSize + x;
		};

		std::queue<glm::ivec2> q;
		q.push(seed);
		
		while (!q.empty())
		{
			glm::ivec2 current = q.front();
			q.pop();

			int index = GI(current.x, current.y, sliceNumber);// boundaries checked on the next line so it will be caught but later
			if (current.x >= 0 && current.y >= 0 && current.x < xSize && current.y < ySize && data[index][contourNumber] == 0)
			{
				data[index][contourNumber] = 1;
				q.push(glm::ivec2(current.x - 1, current.y));
				q.push(glm::ivec2(current.x + 1, current.y));
				q.push(glm::ivec2(current.x, current.y - 1));
				q.push(glm::ivec2(current.x, current.y + 1));
			}
		}
	}
}