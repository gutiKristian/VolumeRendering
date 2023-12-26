#include "DcmImpl.h"
#include <cassert>
#include <string>

#include "Base/Log.h"

namespace med
{
	VolumeFile DcmImpl::readFile(const std::filesystem::path& name, bool isDir)
	{
		// Init
		std::vector<std::filesystem::path> paths;
		if (isDir)
		{
			paths = listDirFiles(name, /*extension=*/".dcm");
		}
		else
		{
			paths.push_back(m_Path / name);
		}

		std::size_t numberOfFiles = paths.size();

		if (numberOfFiles == 0)
		{
			LOG_ERROR("No files found!");
			throw new std::exception("no files");
		}

		bool isFirstRun = true;
		int offset = 0;
		// Reading
		for (const auto& file : paths)
		{
			dcm::DicomFile f(file.c_str());

			if (f.Load())
			{
				initDicomVariables(f);

				// Dir, multi image images are unsupported
				assert(m_NumberOfFrames <= 1 && isDir);
				
				if (isFirstRun)
				{
					if (!allocateMemory(numberOfFiles, isDir))
					{
						LOG_ERROR("Allocation failed");
;						throw std::exception("Couldn't allocate memory");
					}
					isFirstRun = false;
				}

				// Reads data into p_Data
				readData(f, offset);

			}
			else
			{
				LOG_ERROR("Cannot continue, unable to open: ", file.string());
				throw std::exception("Error!");
			}

			//On to another image if exist
			++offset;
		}

		// Create new Volume file (more like data class)
		VolumeFile file(m_Path / name, isDir);
		file.setDataPtr(p_Data);
		file.setFileDataType(m_FileDataType);
		file.setSize({ m_Rows, m_Cols, m_Depth });
		return file;
	}

	void DcmImpl::initDicomVariables(const dcm::DicomFile& f)
	{
		std::uint16_t rows = 0, cols = 0, bitsStored = 0, bitsAllocated = 0; 
		std::int16_t numberOfFrames = 0;

		f.GetUint16(dcm::tags::kRows, &rows);
		f.GetUint16(dcm::tags::kColumns, &cols);
		// Sometimes number of frames is zero, this means one frame 
		f.GetInt16(dcm::tags::kNumberOfFrames, &numberOfFrames);
		f.GetUint16(dcm::tags::kBitsStored, &bitsStored);
		f.GetUint16(dcm::tags::kBitsAllocated, &bitsAllocated);
		
		updateValue(m_Rows, rows, "Rows");
		updateValue(m_Cols, cols, "Cols");
		updateValue(m_BitsStored, bitsStored, "Bits stored");
		updateValue(m_BitsAllocated, bitsAllocated, "Bits allocated");
		updateValue(m_NumberOfFrames, numberOfFrames, "Number of frames");
	}

	void DcmImpl::updateValue(auto& originalVal, auto& newVal, std::string&& tag)
	{
		if (originalVal != 0 && originalVal != newVal)
		{
			LOG_ERROR("Mismatch in tag: ", tag);
			LOG_ERROR("Original value: ", std::to_string(originalVal), ", new value: ", std::to_string(newVal));
			throw std::exception("Mismatch ");
		}
		originalVal = newVal;
	}

	bool DcmImpl::allocateMemory(std::size_t frames, bool isDir)
	{
		std::uint16_t z = 0;
		if (isDir)
		{
			z = frames;
		}
		else
		{
			z = m_NumberOfFrames;
		}

		m_Depth = z;
		const auto memorySize = m_Rows * m_Cols * m_Depth;

		switch (m_BitsAllocated)
		{
		case 16:
		{
			m_FileDataType = FileDataType::Uint16;
			p_Data = new std::uint16_t[memorySize];
			break;
		}
		case 32:
		{
			m_FileDataType = FileDataType::Float32;
			p_Data = new float[memorySize];
			break;
		}
		case 64:
		{
			m_FileDataType = FileDataType::Double;
			p_Data = new double[memorySize];
			break;
		}
		default:
			// Undefined
			p_Data = nullptr;
			return false;
		}

		// When reading a directory, we expect this to be list of dicom files with one frame
		setResolution(m_Rows * m_Cols * (isDir ? 1 : z));
		return true;

	}

	void DcmImpl::readData(const dcm::DicomFile& f, int offset)
	{
		// When reading multiple images
		void* offsetPtr;
		switch (m_FileDataType)
		{
		case med::FileDataType::Uint16:
		{
			std::vector<std::uint16_t> vec;
			f.GetUint16Array(dcm::tags::kPixelData, &vec);
			offsetPtr = static_cast<std::uint16_t*>(p_Data) + (offset * m_Res);
			std::memcpy(offsetPtr, vec.data(), m_Res * sizeof(std::uint16_t));
			assert(vec[vec.size() - 1] == reinterpret_cast<std::uint16_t*>(p_Data)[vec.size() - 1]);
			break;
		}
		case med::FileDataType::Float32:
			break;
		case med::FileDataType::Double:
			break;
		default:
			break;
		}
	}

}