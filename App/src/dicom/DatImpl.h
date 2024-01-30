#pragma once
#include "FileReader.h"
#include "VolumeFile.h"
#include <filesystem>

namespace med
{
	class DatImpl : public FileReader
	{
	public:
		DatImpl() = default;
		~DatImpl() = default;
	public:

		/**
		 * Reads file into memory, inside defaultPath / name
		 */
		VolumeFile readFile(const std::filesystem::path& name, bool isDir) override;
	};
}