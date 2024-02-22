#include "FileReader.h"
#include <fstream>

namespace med
{
	std::filesystem::path FileReader::s_Path = std::filesystem::current_path() / ".." / ".." / ".." / ".." / "App";

	std::string FileReader::ReadFile(const std::filesystem::path& name)
	{
		std::string buffer;
		std::ifstream file(s_Path / name);

		file.seekg(0, std::ios::end);
		buffer.resize(file.tellg());
		file.seekg(0);
		file.read(buffer.data(), buffer.size());

		return buffer;
	}

}