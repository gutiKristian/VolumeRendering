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

        template <typename T>
        size_t GetMaxNumber(const std::vector<T>& vec)
        {
            if constexpr (std::is_same_v<T, glm::vec4>)
            {
                auto maxElement = std::max_element(vec.begin(), vec.end(), [](const T& a, const T& b) {
                    return a.r < b.r;
                });
                return maxElement != vec.end() ? maxElement->r : 0;
            }
            else
            {
                auto maxElement = std::max_element(vec.begin(), vec.end());
                return maxElement != vec.end() ? *maxElement : 0;
            }
        }

		size_t GetMaxUsedBits(size_t maxNumber)
		{
			return std::bit_width(maxNumber);
		}

		bool IsDirectory(const std::filesystem::path& path)
		{
			return std::filesystem::is_directory(path);
		}

	protected:
		static std::filesystem::path s_Path;
	};
}