#pragma once
#include "VolumeFile.h"

#include <filesystem>
#include <cassert>
#include <string>
#include <memory>

namespace med
{
	class FileReader
	{
	protected:
		FileReader() = default;
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
			for (const auto& entry : std::filesystem::directory_iterator(s_Path / path))
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

	protected:
		static std::filesystem::path s_Path;
	};
}