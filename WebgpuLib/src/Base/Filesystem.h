#pragma once

#include <string>
#include <filesystem>
#include <optional>

namespace base {

	class Filesystem
	{
	public:
		std::optional<std::string> ReadFile(const std::filesystem::path& relativeFilepath);
		static void Init();
	private:
		inline static std::filesystem::path s_AssetsPath;
	};



}
