#pragma once

#include "DicomParams.h"

namespace med
{
	// Interface for DICOM file
	class IDicomFile
	{
	public:
		virtual ~IDicomFile() = default;
		
		virtual DicomBaseParams GetBaseParams() const = 0;
		virtual DicomModality GetModality() const = 0;
		virtual bool CompareFrameOfReference(const IDicomFile& other) const = 0;
	};
}