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
			FileDataType type, DicomVolumeParams params, std::vector<glm::vec4>& data) : VolumeFile(path, size, type, data), m_Params(params) {};
	public:
		DicomBaseParams GetBaseParams() const override;
		DicomModality GetModality() const override;
		bool CompareFrameOfReference(const IDicomFile& other) const override;
	private:
		DicomVolumeParams m_Params;
	};
}