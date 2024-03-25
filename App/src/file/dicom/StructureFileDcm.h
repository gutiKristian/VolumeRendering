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
	class StructureFileDcm : public IDicomFile
	{
	public:
		StructureFileDcm(std::filesystem::path path, DicomStructParams params, std::vector<std::vector<std::vector<float>>> data);
		
	public:
		DicomBaseParams GetBaseParams() const override;
		DicomModality GetModality() const override;
		bool CompareFrameOfReference(const IDicomFile& other) const override;
		std::shared_ptr<VolumeFileDcm> Create3DMask(const IDicomFile& other, std::array<int, 4> contourIDs, bool handleDuplicates = false) const;

	private:
		std::filesystem::path m_Path;
		DicomStructParams m_Params;
		std::vector<std::vector<std::vector<float>>> m_Data;
	};
}