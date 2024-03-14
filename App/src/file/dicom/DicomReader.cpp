#include "DicomReader.h"

#include "Base/Log.h"
#include "VolumeFileDcm.h"
#include "dcm/defs.h"
#include "DicomParams.h"

#include <cassert>
#include <string>



namespace med
{
	const dcm::Tag kInstanceNumber = 0x00200013;
	const dcm::Tag kImagePositionPatient = 0x00200013;
	const dcm::Tag kImageOrientationPatient = 0x00200013;
	const dcm::Tag kSliceThickness = 0x00200013;

	std::unique_ptr<VolumeFileDcm> DicomReader::ReadVolumeFile(const std::filesystem::path& name, bool isDir)
	{
		bool firstRun = true;
		m_Data = std::vector<glm::vec4>();
		m_Params = DicomVolumeParams();

		// Init
		std::vector<std::filesystem::path> paths;
		if (isDir)
		{
			paths = SortDicomSlices(ListDirFiles(name, /*extension=*/".dcm"));
		}
		else
		{
			paths.push_back(s_Path / name);
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

		if (isDir)
		{
			m_Params.Z = numberOfFiles;
		}

		auto n = GetMaxNumber<glm::vec4>(m_Data);
		auto bits = GetMaxUsedBits(n);
		std::tuple<std::uint16_t, std::uint16_t, std::uint16_t> size = { m_Params.X, m_Params.Y, m_Params.Z };
		return std::make_unique<VolumeFileDcm>(s_Path / name, size, m_FileDataType, m_Params, m_Data);
	}

	std::unique_ptr<StructureFileDcm> DicomReader::ReadStructFile(const std::filesystem::path& name)
	{
		auto params = DicomStructParams();
		return std::make_unique<StructureFileDcm>(params);
	}

	DicomModality DicomReader::CheckModality(const std::filesystem::path& name)
	{
		dcm::DicomFile f(name.c_str());
		if (f.Load())
		{
			std::string modality;
			f.GetString(dcm::tags::kModality, &modality);
			if (modality == "CT")
				return DicomModality::CT;
			else if (modality == "MR")
				return DicomModality::MR;
			else if (modality == "RTDOSE")
				return DicomModality::RTDOSE;
			else if (modality == "RTSTRUCT")
				return DicomModality::RTSTRUCT;
		}
		return DicomModality::UNKNOWN;
	}

	void DicomReader::ReadDicomVariables(const dcm::DicomFile& f)
	{
		// Lambda functions for parsing
		auto IS = [](std::string& str) -> int
		{
			if (str == "")
			{
				// For dataset spread across multiple files
				return 1;
			}

			// parse int in the string if it fails return 1
			try
			{
				return std::stoi(str);
			}
			catch (std::exception& e)
			{
				// Something strange happened
				LOG_ERROR(("Error parsing string to int: " + std::string(e.what())).c_str());
				return 1;
			}
		};

		// Read the parameters
		DicomVolumeParams currentParams;

		f.GetUint16(dcm::tags::kRows, &currentParams.X);
		f.GetUint16(dcm::tags::kColumns, &currentParams.Y);

		std::string str;
		// Sometimes number of frames is zero, this means one frame 
		f.GetString(dcm::tags::kNumberOfFrames, &str);
		currentParams.Z = IS(str);

		f.GetUint16(dcm::tags::kBitsStored, &currentParams.BitsStored);
		f.GetUint16(dcm::tags::kBitsAllocated, &currentParams.BitsAllocated);
		
		f.GetString(kImageOrientationPatient, &str);
		currentParams.ImageOrientationPatient = DS<6>(str);

		//TODO: We have to keep only the position from the first slice
		f.GetString(kImagePositionPatient, &str);
		currentParams.ImagePositionPatient = DS<3>(str);

		f.GetString(kSliceThickness, &str);
		std::array<double, 1> sT{ 0.0 };
		sT = DS<1>(str);
		currentParams.SliceThickness = sT[0];

		f.GetString(dcm::tags::kPixelSpacing, &str);
		currentParams.PixelSpacing = DS<2>(str);
		
		if (m_Params.X != 0 && m_Params.Y != 0)
		{
			// Do the checks, only if we have data in multiple files
		}

		// All good, update the current parameters
		m_Params = currentParams;

		// Update the file type, based on this information we can allocate memory
		ResolveFileType();

	}

	bool DicomReader::PreAllocateMemory(std::size_t frames, bool isDir)
	{
		// Pre-allocating
		m_Data.reserve(m_Params.X * m_Params.Y * (isDir ? frames : m_Params.Z));

		return true;
	}

	void DicomReader::ReadData(const dcm::DicomFile& f)
	{
		switch (m_FileDataType)
		{
		case FileDataType::Uint16:
		{
			std::vector<std::uint16_t> vec;
			f.GetUint16Array(dcm::tags::kPixelData, &vec);
			assert(vec.size() == (m_Params.X * m_Params.Y * m_Params.Z) && "Expected resolution of image does not match with loaded one");
			std::ranges::transform(vec, std::back_inserter(m_Data), [](auto& value) { return glm::vec4(value); });
			break;
		}
		case FileDataType::Uint32:
		{
			std::vector<std::uint32_t> vec;
			f.GetUint32Array(dcm::tags::kPixelData, &vec);
			assert(vec.size() == (m_Params.X * m_Params.Y * m_Params.Z) && "Expected resolution of image does not match with loaded one");
			std::ranges::transform(vec, std::back_inserter(m_Data), [](auto& value) { return glm::vec4(value); });
			break;
		}
		case FileDataType::Double:
			break;
		default:
			break;
		}
	}

	std::vector<std::filesystem::path> DicomReader::SortDicomSlices(const std::vector<std::filesystem::path>& paths) const
	{
		std::vector< std::pair<int, std::filesystem::path> > pairs;
		std::vector<std::filesystem::path> result;

		// Assigns passed reference slice number, returns false when fails
		auto retrieveOrder = [&](const std::filesystem::path& path) -> void
		{
			std::string value;
			dcm::DicomFile f(path.c_str());
			if (f.Load() && !f.GetString(kInstanceNumber, &value))
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

	void DicomReader::ResolveFileType()
	{
		switch (m_Params.BitsAllocated)
		{
			case 16:
				m_FileDataType = FileDataType::Uint16;
			break;
			case 32:
				m_FileDataType = FileDataType::Uint32;
			break;
			case 64:
				m_FileDataType = FileDataType::Double;
			break;
			default:
				throw std::exception("Unknown type");
		}
	}

}
