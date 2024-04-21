#include "DatReader.h"
#include "Base/Base.h"
#include "../FileSystem.h"

#include <fstream>
#include <string>
#include <sstream>

namespace med
{
	VolumeFile DatImpl::ReadFile(const std::filesystem::path& name, bool isDir)
	{
		using ushort = unsigned short;

		std::ifstream file(FileSystem::GetDefaultPath() / name, std::ios_base::binary);

		if (file.fail() || !file.is_open())
		{
			throw std::exception("Check file");
		}

		// Header is 6bytes long
		static_assert(sizeof(ushort) == 2);

		constexpr int HEADER_SIZE = 6;
		ushort dims[3];
		file.read(reinterpret_cast<char*>(dims), HEADER_SIZE);

		std::stringstream dimsStr;
		dimsStr << dims[0] << " x " << dims[1] << " x " << dims[2] << "\n";

		LOG_INFO("Dat File");
		LOG_INFO(dimsStr.str().c_str());

		const unsigned int res = dims[0] * dims[1] * dims[2];
		assert(res != 0 && "File is empty");

		std::vector<ushort> rawImg(res);
		std::vector<glm::vec4> data(res);

		file.read(reinterpret_cast<char*>(rawImg.data()), res * 2); // times two since one number is two bytes
		std::ranges::transform(rawImg, std::back_inserter(data), [](auto& value) { return glm::vec4(value); });

		// Create new Volume file (more like data class)
		return VolumeFile(FileSystem::GetDefaultPath() / name, { dims[0], dims[1], dims[2] }, FileDataType::Uint16, data);
	}
}