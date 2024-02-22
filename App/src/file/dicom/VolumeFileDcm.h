#pragma once

#include "../VolumeFile.h"
#include "DicomParams.h"

#include <filesystem>

namespace med
{
	class VolumeFileDcm : public VolumeFile
	{

	public:
		VolumeFileDcm(DicomParams params) : m_Params(params) {};
		VolumeFileDcm(const std::filesystem::path& path, DicomParams params) : VolumeFile(path), m_Params(params) {};

	private:
		DicomParams m_Params;
	};
}