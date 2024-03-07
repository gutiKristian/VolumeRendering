#pragma once

#include "../VolumeFile.h"
#include "DicomParams.h"

#include <filesystem>

namespace med
{
	class VolumeFileDcm : public VolumeFile
	{

	public:
		VolumeFileDcm(DicomImageParams params) : m_Params(params) {};
		VolumeFileDcm(const std::filesystem::path& path, DicomImageParams params) : VolumeFile(path), m_Params(params) {};

	private:
		DicomImageParams m_Params;
	};
}