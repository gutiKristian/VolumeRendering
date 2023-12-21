#include "FileReader.h"
#include <fstream>
#include <iostream>
#include <array>

//#UNUSED
namespace med
{





	// Parsing DAT FILE
	//void FileReader::read_file(const std::string& name)
	//{
	//	using ushort = unsigned short;

	//	std::cout << "[FILE READER]: Opening: " << s_Path / name << "\n";

	//	std::ifstream file(s_Path / name, std::ios_base::binary);

	//	if (file.fail() || !file.is_open())
	//	{
	//		std::cout << "[FILE READER]: Couldn't read the file: " << s_Path / name << "\n";
	//		throw new std::exception("Check file");
	//	}

	//	// Header is 6bytes long
	//	static_assert(sizeof(ushort) == 2);

	//	ushort dims[3];
	//	file.read(reinterpret_cast<char*>(dims), 6);
	//	m_dim = { dims[0], dims[1], dims[2] };

	//	std::cout << "Dimensions: [" << dims[0] << ", " << dims[1] << ", " << dims[2] << "]\n";

	//	const unsigned int res = dims[0] * dims[1] * dims[2];
	//	ushort* img = new ushort[res];
	//	file.read(reinterpret_cast<char*>(img), res * 2); // times two since one number is two bytes
	//	m_Data = img;

	//	/*

	//	// Read by slice
	//	ushort** img_array = new unsigned short* [dims[2]];

	//	const auto resolution = dims[0] * dims[1];

	//	// Traverse images
	//	for (size_t i = 0; i < dims[2]; ++i)
	//	{
	//		img_array[i] = new unsigned short[resolution];
	//		file.read(reinterpret_cast<char*>(img_array[i]), resolution * 2); // times two since one number is two bytes

	//		m_data = img_array;
	//	}
	//	*/


	std::string FileReader::readFile(const std::filesystem::path& name)
	{
		std::string buffer;
		std::ifstream file(m_Path / name);

		file.seekg(0, std::ios::end);
		buffer.resize(file.tellg());
		file.seekg(0);
		file.read(buffer.data(), buffer.size());

		return buffer;
	}

	//}
}