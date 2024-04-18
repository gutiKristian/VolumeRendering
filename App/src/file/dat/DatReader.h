#pragma once

#include "../FileSystem.h"
#include "../VolumeFile.h"

#include <filesystem>

namespace med
{
	class DatImpl : public FileSystem
	{
	public:

		/**
		 * Reads file into memory, inside defaultPath / name
		 */
		[[nodiscard]] VolumeFile ReadFile(const std::filesystem::path& name, bool isDir);
	};
}