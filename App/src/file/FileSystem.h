#pragma once
#include "VolumeFile.h"

#include <filesystem>
#include <cassert>
#include <string>
#include <memory>

namespace med
{
	class FileSystem
	{
	protected:
		FileSystem() = default;
	public:
		
		/*
		* Basic util, reads file and returns string. Could be used for shaders.
		*/
		static std::string ReadFile(const std::filesystem::path& name);

		/*
		* List files with extension, inside the dir.
		*/
		[[nodiscard]] static std::vector<std::filesystem::path> ListDirFiles(const std::filesystem::path& path, const std::string& extension)
		{
			assert(extension.starts_with("."));
			
			std::vector<std::filesystem::path> paths;
			for (const auto& entry : std::filesystem::directory_iterator(path))
			{
				if (entry.path().extension().string() == extension)
				{
					paths.push_back(entry.path());
				}
			}
			return paths;
		}

		[[nodiscard]] static std::filesystem::path& GetDefaultPath()
		{
			return s_Path;
		}


		[[nodiscard]] static size_t GetMaxUsedBits(size_t maxNumber)
		{
			return std::bit_width(maxNumber);
		}

		[[nodiscard]] static bool IsDirectory(const std::filesystem::path& path)
		{
			return std::filesystem::is_directory(path);
		}

	protected:
		static std::filesystem::path s_Path;
	};
}