#include "VolumeFileDcm.h"

namespace med
{
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
}