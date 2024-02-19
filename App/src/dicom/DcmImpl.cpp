#include "DcmImpl.h"
#include <cassert>
#include <string>

#include "Base/Log.h"

namespace med
{
	VolumeFile DcmImpl::ReadFile(const std::filesystem::path& name, bool isDir)
	{
		// Init
		std::vector<std::filesystem::path> paths;
		if (isDir)
		{
			paths = SortDicomSlices(ListDirFiles(name, /*extension=*/".dcm"));
		}
		else
		{
			paths.push_back(m_Path / name);
		}

		std::size_t numberOfFiles = paths.size();

		if (numberOfFiles == 0)
		{
			LOG_ERROR("No files found!");
			throw new std::exception("No dicom files found!");
		}

		// Reading
		for (const auto& file : paths)
		{
			dcm::DicomFile f(file.c_str());

			if (f.Load())
			{
				ReadDicomVariables(f);

				static bool firstRun = true;
				if (firstRun)
				{
					PreAllocateMemory(numberOfFiles, isDir);
				}
				firstRun = false;

				ReadData(f);
			}
			else
			{
				LOG_ERROR("Cannot continue, unable to open: ", file.string());
				throw std::exception("Error!");
			}
		}

		// Create new Volume file (data class)
		VolumeFile file(m_Path / name, isDir);
		file.SetData(m_Data);
		file.SetFileDataType(m_FileDataType);
		file.SetDataSize({ m_Rows, m_Cols, m_Depth });
		return file;
	}

	void DcmImpl::ReadDicomVariables(const dcm::DicomFile& f)
	{
		std::uint16_t rows = 0, cols = 0, bitsStored = 0, bitsAllocated = 0; 
		std::int16_t numberOfFrames = 0;

		f.GetUint16(dcm::tags::kRows, &rows);
		f.GetUint16(dcm::tags::kColumns, &cols);
		// Sometimes number of frames is zero, this means one frame 
		f.GetInt16(dcm::tags::kNumberOfFrames, &numberOfFrames);
		f.GetUint16(dcm::tags::kBitsStored, &bitsStored);
		f.GetUint16(dcm::tags::kBitsAllocated, &bitsAllocated);
		
		CompareAndUpdateValue(m_Rows, rows, "Rows");
		CompareAndUpdateValue(m_Cols, cols, "Cols");
		CompareAndUpdateValue(m_BitsStored, bitsStored, "Bits stored");
		CompareAndUpdateValue(m_BitsAllocated, bitsAllocated, "Bits allocated");
		CompareAndUpdateValue(m_NumberOfFrames, numberOfFrames, "Number of frames");
		ResolveFileType();
	}

	void DcmImpl::CompareAndUpdateValue(auto& originalVal, auto& newVal, std::string&& tag)
	{
		if (originalVal != 0 && originalVal != newVal)
		{
			LOG_ERROR("Mismatch in tag: ", tag);
			LOG_ERROR("Original value: ", std::to_string(originalVal), ", new value: ", std::to_string(newVal));
			throw std::exception("Mismatch ");
		}
		originalVal = newVal;
	}

	bool DcmImpl::PreAllocateMemory(std::size_t frames, bool isDir)
	{
		// 3D data can be represented as 2D array or one 3D file
		m_Depth = isDir ? frames : m_NumberOfFrames;

		// Pre-allocating
		m_Data.reserve(m_Rows * m_Cols * m_Depth);

		// When reading a directory, we expect this to be list of dicom files with one frame
		SetResolution(m_Rows * m_Cols * (isDir ? 1 : m_Depth));

		return true;
	}

	void DcmImpl::ReadData(const dcm::DicomFile& f)
	{
		switch (m_FileDataType)
		{
		case FileDataType::Uint16:
		{
			std::vector<std::uint16_t> vec;
			f.GetUint16Array(dcm::tags::kPixelData, &vec);
			assert(vec.size() == m_Res && "Expected resolution of image does not match with loaded one");
			std::ranges::transform(vec, std::back_inserter(m_Data), [](auto& value) { return glm::vec4(value); });
			break;
		}
		case FileDataType::Float32:
			break;
		case FileDataType::Double:
			break;
		default:
			break;
		}
	}

	std::vector<std::filesystem::path> DcmImpl::SortDicomSlices(const std::vector<std::filesystem::path>& paths) const
	{
		std::vector< std::pair<int, std::filesystem::path> > pairs;
		std::vector<std::filesystem::path> result;

		// Assigns passed reference slice number, returns false when fails
		auto retrieveOrder = [&](const std::filesystem::path& path) -> void
		{
			std::string value;
			dcm::DicomFile f(path.c_str());
			if (f.Load() && !f.GetString(dcm::tags::kInstanceNumber, &value))
			{
				LOG_ERROR("Error missing instnace number in dicom file");
				return;
			}
			int order = std::stoi(value);
			pairs.emplace_back(order, path);
		};

		// Initialize each file with its relative position within the directory
		std::ranges::for_each(paths, retrieveOrder);

		if (paths.size() != pairs.size())
		{
			LOG_WARN("Default order of path is going to be used.");
			return paths;
		}

		// Sort them in ascending order
		std::ranges::sort(pairs, [](const auto& pair1, const auto& pair2) { return pair1.first < pair2.first; });
		// Transform it to the final result
		std::ranges::transform(pairs, std::back_inserter(result), [](const auto& pair) {return pair.second; });

		LOG_TRACE("DICOM sorting success");
		return result;
	}

	void DcmImpl::ResolveFileType()
	{
		switch (m_BitsAllocated)
		{
			case 16:
				m_FileDataType = FileDataType::Uint16;
			break;
			case 32:
				m_FileDataType = FileDataType::Float32;
			break;
			case 64:
				m_FileDataType = FileDataType::Double;
			break;
			default:
				throw std::exception("Unknown type");
		}
	}
}
