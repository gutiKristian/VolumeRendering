#include "DatImpl.h"

#include "Base/Base.h"

#include <fstream>
#include <string>
#include <sstream>

namespace med
{
	VolumeFile DatImpl::readFile(const std::filesystem::path& name, bool isDir)
	{
		using ushort = unsigned short;

		std::ifstream file(m_Path / name, std::ios_base::binary);

		if (file.fail() || !file.is_open())
		{
			throw std::exception("Check file");
		}

		// Header is 6bytes long
		static_assert(sizeof(ushort) == 2);

		ushort dims[3];
		file.read(reinterpret_cast<char*>(dims), 6);
		LOG_INFO("Dat File");
		std::stringstream dimsStr;
		dimsStr << dims[0] << " x " << dims[1] << " x " << dims[2] << "\n";
		LOG_INFO(dimsStr.str().c_str());

		const unsigned int res = dims[0] * dims[1] * dims[2];
		ushort* img = new ushort[res];
		file.read(reinterpret_cast<char*>(img), res * 2); // times two since one number is two bytes

		// Create new Volume file (more like data class)
		VolumeFile volFile(m_Path / name, isDir);
		volFile.setDataPtr(img);
		volFile.setFileDataType(FileDataType::Uint16);
		volFile.setSize({ dims[0], dims[1], dims[2]});
		return volFile;

	}
}