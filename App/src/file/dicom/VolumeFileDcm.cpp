#include "VolumeFileDcm.h"
#include "Base/Log.h"
#include "../FileSystem.h"

#include <glm/glm.hpp>

namespace med
{
	VolumeFileDcm::VolumeFileDcm(std::filesystem::path path, std::tuple<std::uint16_t, std::uint16_t, std::uint16_t> size, 
		FileDataType type, DicomVolumeParams params, std::vector<glm::vec4>& data) : VolumeFile(path, size, type, data, params.LargestPixelValue), m_Params(params)
	{
		InitializeTransformMatrices();
		CalcMainAxis();
		m_CustomBitWidth = FileSystem::GetMaxUsedBits(m_MaxNumber);

		// For now suppress ranges higher than 12 bits as these values are barely reached anyway
		// and even 12bit is impossible to capture on many monitors
		if (m_Params.Modality != DicomModality::RTDOSE && m_CustomBitWidth > 12)
		{
			m_CustomBitWidth = 12;
		}

		m_MaxNumber = (1 << m_CustomBitWidth) - 1;
	}

	DicomBaseParams VolumeFileDcm::GetBaseParams() const
	{
		return m_Params;
	}

	DicomModality VolumeFileDcm::GetModality() const
	{
		return m_Params.Modality;
	}

	bool VolumeFileDcm::CompareFrameOfReference(const IDicomFile& other) const
	{
		return other.GetBaseParams().FrameOfReference == m_Params.FrameOfReference;
	}

	bool VolumeFileDcm::CompareOrientation(const VolumeFileDcm& other) const
	{
		auto otherOrientation = other.GetVolumeParams().ImageOrientationPatient;
		int res = 0;
		for (int i = 0; i < otherOrientation.size(); ++i)
		{
			res += otherOrientation[i] * m_Params.ImageOrientationPatient[i];
		}
		return res != 0;
	}

	DicomVolumeParams VolumeFileDcm::GetVolumeParams() const
	{
		return m_Params;
	}

	glm::vec3 VolumeFileDcm::PixelToRCSTransform(glm::vec2 coord) const
	{
		glm::vec4 res = m_PixelToRCS * glm::vec4(coord.x, coord.y, 0.0f, 1.0f);
		return glm::vec3(res.x, res.y, res.z);
	}

	glm::vec2 VolumeFileDcm::RCSToPixelTransform(glm::vec3 coord) const
	{
		glm::vec4 res = m_RCSToPixel * glm::vec4(coord, 1.0f);
		return glm::vec2(res.x, res.y);
	}

	glm::vec3 VolumeFileDcm::RCSToVoxelTransform(glm::vec3 coord) const
	{
		glm::vec4 res = m_RCSToPixel * glm::vec4(coord, 1.0f);
		res.z /= m_Params.SliceThickness;
		return glm::vec3(res.x, res.y, res.z);
	}

	void VolumeFileDcm::SetContourSliceNumbers(std::vector<std::vector<int>> sliceNumbers)
	{
		for (const auto& vec : sliceNumbers)
		{
			std::map<int, int> objectCount{};

			for (const auto& i : vec)
			{
				objectCount[i]++;
			}
			m_CtrSliceNum.push_back(objectCount);
		}
	}

	void VolumeFileDcm::InitializeTransformMatrices()
	{
		// Pixel spacing array order is rows, cols
		const auto [dj, di] = m_Params.PixelSpacing;
		const auto [Sx, Sy, Sz] = m_Params.ImagePositionPatient;
		const auto [Xx, Xy, Xz, Yx, Yy, Yz] = m_Params.ImageOrientationPatient;

		// Create the matrix
		/*
		(Xx * di)  (Yx * dj)  (0)  (Sx)
		(Xy * di)  (Yy * dj)  (0)  (Sy)
		(Xz * di)  (Yz * dj)  (0)  (Sz)
		(   0   )  (   0   )  (0)  (1)
		*/
		glm::mat4x4 T = glm::mat4x4(1.0f);
		T[0][0] = Xx * di;
		T[1][0] = Yx * dj;
		T[3][0] = Sx;
		T[0][1] = Xy * di;
		T[1][1] = Yy * dj;
		T[3][1] = Sy;
		T[0][2] = Xz * di;
		T[1][2] = Yz * dj;
		T[3][2] = Sz;

		m_PixelToRCS = T;
		m_RCSToPixel = glm::inverse(T);
	}

	void VolumeFileDcm::CalcMainAxis()
	{
		auto thisOrientation = m_Params.ImageOrientationPatient;

		glm::vec3 rowDir(thisOrientation[0], thisOrientation[1], thisOrientation[2]);
		glm::vec3 colDir(thisOrientation[3], thisOrientation[4], thisOrientation[5]);

		glm::vec3 res = glm::cross(rowDir, colDir);

		if (res == glm::vec3(0, 0, 0))
		{
			m_Params.MainAxis = "X";
		}
		else if (res == glm::vec3(0, 1, 0))
		{
			m_Params.MainAxis = "Y";
		}
		else if (res == glm::vec3(0, 0, 1))
		{
			m_Params.MainAxis = "Z";
		}
		else
		{
			// ???
			LOG_CRITICAL("Calculate Main Axis, unexpected result!");
		}
		
	}
}