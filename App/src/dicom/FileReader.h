#pragma once
#include "VolumeFile.h"
#include <filesystem>
#include <cassert>
#include <string>

namespace med
{
	class FileReader
	{
	public:
		FileReader() = default;
		virtual ~FileReader() = default;
	public:

		/*
		* Basic util, reads file and returns string. Could be used for shaders.
		*/
		std::string ReadFile(const std::filesystem::path& name);

	protected:

		/*
		* Reads file or directory of files into memory. Path: defaultPath / name
		*/
		virtual VolumeFile ReadFile(const std::filesystem::path& name, bool isDir) { throw std::exception("Volume file reading is not implemented in basic FileReader!\n"); };

		/*
		* List files with extension, inside the dir.
		*/
		[[nodiscard]] std::vector<std::filesystem::path> ListDirFiles(const std::filesystem::path& path, const std::string& extension) const
		{
			assert(extension.starts_with("."));
			
			std::vector<std::filesystem::path> paths;
			for (const auto& entry : std::filesystem::directory_iterator(m_Path / path))
			{
				if (entry.path().extension().string() == extension)
				{
					paths.push_back(entry.path());
				}
			}
			return paths;
		}

	public:

		[[nodiscard]] const std::filesystem::path& getDefaultPath() const
		{
			return m_Path;
		}

		/*
		 * Set new default path for opening assets
		 */
		void setDefaultPath(std::filesystem::path new_default_path)
		{
			m_Path = new_default_path;
		}

	protected:
		std::filesystem::path m_Path = std::filesystem::current_path() / ".." / ".." / ".." / ".." / "App";
		std::string m_LogPrefix = "[FILE READER]";
		const std::string LOG_EMPTY_PLACEHOLDER = "[[Empty]]";
	};
}