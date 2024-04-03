#pragma once

#include "../VolumeFile.h"
#include "DicomParams.h"
#include "IDicomFile.h"
#include "glm/glm.hpp"

#include <filesystem>
#include <vector>

namespace med
{
	class VolumeFileDcm : public IDicomFile, public VolumeFile
	{
	public:
		VolumeFileDcm(std::filesystem::path path, std::tuple<std::uint16_t, std::uint16_t, std::uint16_t> size,
			FileDataType type, DicomVolumeParams params, std::vector<glm::vec4>& data);
	public:
		DicomBaseParams GetBaseParams() const override;
		DicomModality GetModality() const override;
		bool CompareFrameOfReference(const IDicomFile& other) const override;

		DicomVolumeParams GetVolumeParams() const;

		glm::vec3 PixelToRCSTransform(glm::vec2 coord) const;
		glm::vec2 RCSToPixelTransform(glm::vec3 coord) const;

	private:
		void InitializeTransformMatrices();

	private:
		glm::mat4x4 m_PixelToRCS{ 1.0f };
		glm::mat4x4 m_RCSToPixel{ 1.0f };
		DicomVolumeParams m_Params;
	};
}