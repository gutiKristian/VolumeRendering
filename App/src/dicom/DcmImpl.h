#pragma once
#include "FileReader.h"
#include "dcm/dicom_file.h"
#include "VolumeFile.h"
#include <filesystem>

namespace med
{
	class DcmImpl : public FileReader
	{
	public:
		DcmImpl() { m_LogPrefix = "[DCMIMPL]"; }
		~DcmImpl() = default;
	public:

		/**
		 * Reads file into memory, inside defaultPath / name
		 */
		VolumeFile readFile(const std::filesystem::path& name, bool isDir) override;

	private:
		/*
		* Reads variables from Dicom file.
		* Number of rows, columns, frames, allocated and stored bits.
		*/
		void initDicomVariables(const dcm::DicomFile&);
		/*
		* When loading dir, this method make sure all files are the same width, height,
		* type and so on.
		*/
		void updateValue(auto& originalVal, auto& newVal, std::string&& tag);
		/*
		* Based on detected type prepares memory for entire Volume.
		*/
		bool allocateMemory(std::size_t frames, bool isDir);
		/*
		* Reads data into allocated memory.
		*/
		void readData(const dcm::DicomFile& f, int offset);
		/*
		* Helper, set resolution is used for offsetting data in the memory,
		* if we are trying to load eighty 512x512 images, res = 512x512, and
		* we for every write operation we offset pointer by offset * res e.g. fifth image -> (4 * res)
		*/
		void setResolution(std::uint32_t res) { m_Res = res; }
	private:
		std::uint16_t m_Rows = 0, m_Cols = 0,
			m_BitsStored = 0, m_BitsAllocated = 0;
		std::int16_t m_NumberOfFrames = 0;
		std::uint32_t m_Res = 0;
		std::uint16_t m_Depth = 0;
		FileDataType m_FileDataType = FileDataType::Undefined;
		// Data is transfered to the VolumeFile, which deallocates it
		// Maybe in the future make this already a shared_ptr
		void* p_Data = nullptr;
	};
}