#include "StructureFileDcm.h"

namespace med
{
	StructureFileDcm::StructureFileDcm(DicomStructParams params, std::vector<std::vector<std::vector<float>>> data) : m_Params(params), m_Data(data)
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
}