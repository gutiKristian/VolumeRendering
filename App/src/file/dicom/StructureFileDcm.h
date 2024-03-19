#pragma once

#include "IDicomFile.h"
#include "DicomParams.h"

#include <vector>

namespace med
{
	class StructureFileDcm : public IDicomFile
	{
	public:
		StructureFileDcm(DicomStructParams params, std::vector<std::vector<std::vector<float>>> data);
		
	public:
		DicomBaseParams GetBaseParams() const override;
		DicomModality GetModality() const override;
		bool CompareFrameOfReference(const IDicomFile& other) const override;

	private:
		DicomStructParams m_Params;
		std::vector<std::vector<std::vector<float>>> m_Data;
	};
}