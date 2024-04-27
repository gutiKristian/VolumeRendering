#include "DicomReader.h"

#include "Base/Log.h"
#include "VolumeFileDcm.h"
#include "StructureFileDcm.h"
#include "dcm/defs.h"
#include "DicomParams.h"
#include "DicomParseUtil.h"
#include "StructVisitor.h"
#include "../FileSystem.h"


#include <cassert>
#include <string>
#include <cctype>

namespace med
{
	const dcm::Tag kInstanceNumber = 0x00200013;
	const dcm::Tag kImagePositionPatient = 0x00200032;
	const dcm::Tag kImageOrientationPatient = 0x00200037;
	const dcm::Tag kSliceThickness = 0x00180050;
	const dcm::Tag kFrameOfReference = 0x00200052;
	const dcm::Tag kLargestPixelValue = 0x00280107;
	const dcm::Tag kSmallestPixelValue = 0x00280106;



	std::shared_ptr<VolumeFileDcm> DicomReader::ReadVolumeFile(std::filesystem::path name)
	{
		DicomReader reader;
		bool firstRun = true; // First file

		// Full path
		name = FileSystem::GetDefaultPath() / name;
		
		// Init
		bool isDir = FileSystem::IsDirectory(name);
		std::vector<std::filesystem::path> paths;

		if (isDir)
		{
			paths = SortDicomSlices(FileSystem::ListDirFiles(name, /*extension=*/".dcm"));
		}
		else
		{
			if (!IsDicomFile(name))
			{
				LOG_ERROR("File is not a dicom file!");
				throw std::exception("Error!");
			}

			paths.push_back(name);
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
				if (firstRun)
				{
					reader.ReadDicomVolumeVariables(f);
					reader.m_Params.Modality = ResolveModality(f.GetString(dcm::tags::kModality));
					reader.PreAllocateMemory(numberOfFiles, isDir);
				}
				firstRun = false;

				reader.ReadData(f);
			}
			else
			{
				LOG_ERROR("Cannot continue, unable to open: ", file.string());
				throw std::exception("Error!");
			}
		}

		if (isDir)
		{
			reader.m_Params.Z = numberOfFiles;
		}

		std::tuple<std::uint16_t, std::uint16_t, std::uint16_t> size = { reader.m_Params.X, reader.m_Params.Y, reader.m_Params.Z };
		return std::make_shared<VolumeFileDcm>(FileSystem::GetDefaultPath() / name, size, reader.m_FileDataType, reader.m_Params, reader.m_Data);
	}

	std::shared_ptr<StructureFileDcm> DicomReader::ReadStructFile(std::filesystem::path name)
	{
		name = FileSystem::GetDefaultPath() / name;
		// Handling directory and file extension
		if (FileSystem::IsDirectory(name))
		{
			auto files = FileSystem::ListDirFiles(name, ".dcm");
			
			if (files.empty())
			{
				LOG_ERROR("No structure files found!");
				return nullptr;
			}

			if (files.size() > 1)
			{
				LOG_ERROR("Multiple contour files are not allowed!");
				return nullptr;
			}

			name = files[0];
		}

		if (!IsDicomFile(name))
		{
			LOG_ERROR("File is not a dicom file!");
			return nullptr;
		}

		dcm::DicomFile f(name.c_str());
		
		if (!f.Load())
		{
			std::string err = "Cannot continue, unable to open: " + name.string();
			LOG_ERROR(err.c_str());
			return nullptr;
		}

		// We so support only RTSTRUCT files
		if (ResolveModality(f.GetString(dcm::tags::kModality)) != DicomModality::RTSTRUCT)
		{
			LOG_ERROR("Not a valid struct file");
			return nullptr;
		}
		
		StructVisitor visitor;
		f.Accept(visitor);

		return std::make_shared<StructureFileDcm>(name, visitor.Params, visitor.ContourData);
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


	void DicomReader::ReadDicomVolumeVariables(const dcm::DicomFile& f)
	{
		// String: AE, AS, CS, DA, TM, DT, DS, IS, LO, ST, LT, UT, PN, SH, UC, UI, UR,
		// Read the parameters
		DicomVolumeParams currentParams;

		currentParams.FrameOfReference = f.GetString(kFrameOfReference);

		currentParams.Modality = ResolveModality(f.GetString(dcm::tags::kModality));

		f.GetUint16(dcm::tags::kRows, &currentParams.X);
		f.GetUint16(dcm::tags::kColumns, &currentParams.Y);
		
		std::string str;
		std::vector<std::string> strArr;

		f.GetString(dcm::tags::kNumberOfFrames, &str);
		currentParams.Z = ParseStringToNumArr<int, 1>(str)[0];
		// Usually, the number of frames is 0 when it is one frame
		if (currentParams.Z == 0)
		{
			currentParams.Z = 1;
		}

		f.GetUint16(dcm::tags::kBitsStored, &currentParams.BitsStored);
		f.GetUint16(dcm::tags::kBitsAllocated, &currentParams.BitsAllocated);
	
		f.GetString(kImageOrientationPatient, &str);
		currentParams.ImageOrientationPatient = ParseStringToNumArr<double, 6>(str);

		//TODO: We have to keep only the position from the first slice
		f.GetString(kImagePositionPatient, &str);
		currentParams.ImagePositionPatient = ParseStringToNumArr<double, 3>(str);

		f.GetString(kSliceThickness, &str);
		currentParams.SliceThickness = ParseStringToNumArr<double, 1>(str)[0];

		f.GetString(dcm::tags::kPixelSpacing, &str);
		currentParams.PixelSpacing = ParseStringToNumArr<double, 2>(str);

		std::uint16_t a = 0;
		f.GetUint16(kLargestPixelValue, &currentParams.LargestPixelValue);
		f.GetUint16(kSmallestPixelValue, &currentParams.SmallestPixelValue);

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

	std::vector<std::filesystem::path> DicomReader::SortDicomSlices(const std::vector<std::filesystem::path>& paths)
	{
		if (paths.size() < 2)
		{
			return paths;
		}

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

	bool DicomReader::IsDicomFile(const std::filesystem::path& path)
	{
		return path.extension() == ".dcm";
	}
	
	std::string DicomReader::ResolveModality(DicomModality modality)
	{
		switch (modality)
		{
			case DicomModality::CT:
				return "CT";
			case DicomModality::RTSTRUCT:
				return "RTSTRUCT";
			case DicomModality::RTDOSE:
				return "RTDOSE";
			case DicomModality::MR:
				return "MR";
			default:
				return "UNKNOWN";
		}
	}

	DicomModality DicomReader::ResolveModality(std::string modality)
	{
		std::ranges::transform(modality, modality.begin(), ::toupper);
		if (modality == "CT") return DicomModality::CT;
		if (modality == "RTSTRUCT") return DicomModality::RTSTRUCT;
		if (modality == "RTDOSE") return DicomModality::RTDOSE;
		if (modality == "MR") return DicomModality::MR;
		return DicomModality::UNKNOWN;
	}
}
