#include "Filesystem.h"

#include <fstream>

namespace base {

	std::optional<std::string> Filesystem::ReadFile(const std::filesystem::path& relativeFilepath)
	{
		std::ifstream file(s_AssetsPath / relativeFilepath);
		if (!file.is_open())
		{
			return {};
		}

		std::string buffer;
		file.seekg(0, std::ios::end);
		buffer.resize(file.tellg());
		file.seekg(0);
		file.read(buffer.data(), buffer.size());

		return buffer;
	}

}
