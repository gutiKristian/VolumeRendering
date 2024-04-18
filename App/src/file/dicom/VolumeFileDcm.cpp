#include "VolumeFileDcm.h"

namespace med
{
	VolumeFileDcm::VolumeFileDcm(std::filesystem::path path, std::tuple<std::uint16_t, std::uint16_t, std::uint16_t> size, 
		FileDataType type, DicomVolumeParams params, std::vector<glm::vec4>& data) : VolumeFile(path, size, type, data), m_Params(params)
	{
		InitializeTransformMatrices();
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
}