#include "VolumeFile.h"
#include "Base/Base.h"

namespace med
{
	VolumeFile::VolumeFile(std::filesystem::path path, std::tuple<std::uint16_t, std::uint16_t, std::uint16_t> size, FileDataType type, std::vector<glm::vec4>& data) : 
		m_Path(std::move(path)), m_Size(size), m_FileDataType(type), m_Data(std::move(data))
	{
		m_MaxNumber = GetMaxNumber(m_Data);
	}

	VolumeFile::VolumeFile(std::filesystem::path path, std::tuple<std::uint16_t, std::uint16_t, std::uint16_t> size, FileDataType type, std::vector<glm::vec4>& data, size_t maxNumber) :
		m_Path(std::move(path)), m_Size(size), m_FileDataType(type), m_Data(std::move(data)), m_MaxNumber(maxNumber)
	{
		if (m_MaxNumber == 0)
		{
			m_MaxNumber = GetMaxNumber(m_Data);
		}
	}


	void VolumeFile::SetData(std::vector<glm::vec4> &src)
	{
		m_Data = std::move(src);
	}

	void VolumeFile::SetFileDataType(FileDataType type)
	{
		m_FileDataType = type;
	}

	void VolumeFile::SetDataSize(std::tuple<std::uint16_t, std::uint16_t, std::uint16_t> size)
	{
		m_Size = size;
	}

	void VolumeFile::SetPath(std::filesystem::path path)
	{
		m_Path = std::move(path);
	}

	void VolumeFile::SetMaxNumber(size_t maxNumber)
	{
		m_MaxNumber = maxNumber;
	}

	size_t VolumeFile::GetMaxNumber(const std::vector<glm::vec4>& vec, int index) const
	{
		const std::vector<glm::vec4>::const_iterator maxElement = std::max_element(vec.begin(), vec.end(), [&](const glm::vec4& a, const glm::vec4& b) {
			return a[index] < b[index];
		});

		return maxElement != vec.end() ? (*maxElement)[index] : 0;
	}

	size_t VolumeFile::GetMaxNumber() const
	{
		return m_MaxNumber;
	}

	int VolumeFile::GetMaxUsedBitDepth() const
	{
		return m_CustomBitWidth;
	}

	void VolumeFile::PreComputeGradientSobel()
	{
		if (m_HasGradient)
		{
			LOG_WARN("Gradient has been already computed, skipping...");
			return;
		}

		LOG_INFO("Computing gradients");
		auto [xS, yS, zS] = m_Size;

		for (int z = 1; z < zS - 1; ++z)
		{
			for (int y = 1; y < yS - 1; ++y)
			{
				for (int x = 1; x < xS - 1; ++x)
				{
					float gx = 0.0f, gy = 0.0f, gz = 0.0f;
					for (int i = -1; i <= 1; ++i)
					{
						for (int j = -1; j <= 1; ++j)
						{
							for (int k = -1; k <= 1; ++k)
							{
								gx += GetVoxelData(x + i, y + j, z + k).a * m_SobelX[k + 1][j + 1][i + 1];
								gy += GetVoxelData(x + i, y + j, z + k).a * m_SobelY[k + 1][j + 1][i + 1];
								gz += GetVoxelData(x + i, y + j, z + k).a * m_SobelZ[k + 1][j + 1][i + 1];
							}
						}
					}
					int c = GetIndexFrom3D(x, y, z);
					m_Data[c].x = gx;
					m_Data[c].y = gy;
					m_Data[c].z = gz;
				}
			}
		}
		m_HasGradient = true;
		LOG_INFO("Done");
	}

	void VolumeFile::AverageGradient(int kernelSize)
	{
		LOG_INFO("Averaging gradients");
		
		if (!m_HasGradient)
		{
			LOG_WARN("Gradient not computed.");
			PreComputeGradient();
		}
		kernelSize = (kernelSize - 1) / 2;
		auto [xS, yS, zS] = m_Size;

		// Data traverse
		for (int z = 0; z < zS; ++z)
		{
			for (int y = 0; y < yS; ++y)
			{
				for (int x = 0; x < xS; ++x)
				{
					glm::vec3 sum{ 0.0f };
					int count = 0;
					// Convolution
					for (int k = z - kernelSize; k <= z + kernelSize; k++)
					{
						for (int j = y - kernelSize; j <= y + kernelSize; j++)
						{
							for (int i = x - kernelSize; i <= x + kernelSize; i++)
							{
								int c = GetIndexFrom3D(x, y, z);
								if (c != -1)
								{
									sum.r = m_Data[c].r;
									sum.g = m_Data[c].g;
									sum.b = m_Data[c].b;
									++count;
								}
							}
						}
					}
					// End conv
					sum /= static_cast<float>(count);
				}
			}
		}
	}

	void VolumeFile::NormalizeData()
	{
		if (m_IsNormalized)
			return;
		for (auto& vec : m_Data)
		{
			vec.a /= GetMaxNumber();
		}
		m_IsNormalized = true;
	}

	bool VolumeFile::IsNormalized() const
	{
		return m_IsNormalized;
	}

	bool VolumeFile::HasGradient() const
	{
		return m_HasGradient;
	}

	void VolumeFile::PreComputeGradient(bool normToZeroOne)
	{
		if (m_HasGradient)
		{
			LOG_WARN("Gradient has been already computed, skipping...");
			return;
		}
		
		float maxGradMag = 0.0f;

		LOG_INFO("Computing gradients");
		auto[xS,yS,zS] = m_Size;

		for (int z = 0; z < zS; ++z)
		{
			for (int y = 0; y < yS; ++y)
			{
				for (int x = 0; x < xS; ++x)
				{
					// Index of point where we are computing the gradient
					int c = GetIndexFrom3D(x, y, z);

					glm::vec3 p(0.0f), m(0.0f);
					// X direction
					m.x = GetVoxelData(x - 1, y, z).a;
					p.x = GetVoxelData(x + 1, y, z).a;
					// Y direciton
					m.y = GetVoxelData(x, y - 1, z).a;
					p.y = GetVoxelData(x, y + 1, z).a;
					// Z direciton
					m.z =  GetVoxelData(x, y, z - 1).a;
					p.z = GetVoxelData(x, y, z + 1).a;
					glm::vec3 temp = (-(p - m)) * glm::vec3(0.5); // 1/2h,  h = 1
					if (normToZeroOne)
					{
						float mag = glm::length(temp);
						if (mag > maxGradMag)
						{
							maxGradMag = mag;
						}
					}
					m_Data[c].x = temp.x;
					m_Data[c].y = temp.y;
					m_Data[c].z = temp.z;
				}
			}
		}

		if (normToZeroOne)
		{
			LOG_INFO("Normalizing gradients to [0,1]");
			for (auto& v : m_Data)
			{
				v.x /= maxGradMag;
				v.y /= maxGradMag;
				v.z /= maxGradMag;
			}
		}

		m_HasGradient = true;
		LOG_INFO("Done");
	}

	std::tuple<std::uint16_t, std::uint16_t, std::uint16_t> VolumeFile::GetSize() const
	{
		return m_Size;
	}

	FileDataType VolumeFile::GetFileType() const
	{
		return m_FileDataType;
	}

	const void * VolumeFile::GetVoidPtr() const
	{
		return m_Data.data();
	}

	const std::vector<glm::vec4>& VolumeFile::GetVecReference() const
	{
		return m_Data;
	}

	int VolumeFile::GetIndexFrom3D(int x, int y, int z) const
	{
		auto checkBounds = [&](int& val, const int& upperBound) -> void
		{
			if (val < 0 || val >= upperBound)
				val = -1;
		};

		const auto&[width, height, depth] = m_Size;

		checkBounds(x, width);
		checkBounds(y, height);
		checkBounds(z, depth);

		if (x == -1 || y == -1 || z == -1)
		{
			return -1;
		}

		return z * height * width + y * width + x;
	}

	glm::vec4 VolumeFile::GetVoxelData(int x, int y, int z) const
	{
		int index = GetIndexFrom3D(x, y, z);
		if (index != -1)
		{
			return m_Data[index];
		}
		return glm::vec4(0.0f);
	}
}
