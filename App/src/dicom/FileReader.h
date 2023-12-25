#pragma once
#include "FileDataType.h"
#include "VolumeFile.h"
#include <filesystem>
#include <cassert>
#include <iostream>
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
		std::string readFile(const std::filesystem::path& name);

	protected:

		/*
		* Reads file or directory of files into memory. Path: defaultPath / name
		*/
		virtual VolumeFile readFile(const std::filesystem::path& name, bool isDir) { throw std::exception("Volume file reading is not implemented in basic FileReader!\n"); };

		/*
		* List files with extension, inside the dir.
		*/
		std::vector<std::filesystem::path> listDirFiles(const std::filesystem::path& path, const std::string& extension)
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

		void log(const std::string& s)
		{
			std::cout << "\033[1;32m" + m_LogPrefix + ":\033[0m " << (s == "" ? LOG_EMPTY_PLACEHOLDER : s) << "\n";
		}

		void log(std::stringstream& stream)
		{
			std::string line;
			while (std::getline(stream, line, '\n'))
			{
				log(line);
			}
		}

	public:

		const std::filesystem::path& getDefaultPath() const
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