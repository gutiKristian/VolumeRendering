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
		DcmImpl() = default;
		~DcmImpl() override = default;
	public:

		/**
		 * Reads file into memory, inside defaultPath / name
		 */
		[[nodiscard]] VolumeFile ReadFile(const std::filesystem::path& name, bool isDir) override;

	private:
		/*
		* Reads variables from Dicom file.
		* Number of rows, columns, frames, allocated and stored bits.
		* Checks are done within call of this function (in  CompareAndUpadateValue).
		*/
		void ReadDicomVariables(const dcm::DicomFile&);
		/*
		* When loading dir, this method make sure all files are the same width, height,
		* type and so on.
		*/
		void CompareAndUpdateValue(auto& originalVal, auto& newVal, std::string&& tag);
		/*
		* Based on detected type prepares memory for entire Volume.
		*/
		bool PreAllocateMemory(std::size_t frames, bool isDir);
		/*
		* Reads data into allocated memory.
		*/
		void ReadData(const dcm::DicomFile& f);
		/*
		* Helper, set resolution is used for offsetting data in the memory,
		* if we are trying to load eighty 512x512 images, res = 512x512, and
		* we for every write operation we offset pointer by offset * res e.g. fifth image -> (4 * res)
		*/
		void SetResolution(std::uint32_t res) { m_Res = res; }
		/*
		 * Called during the loading phase to ensure the files are in the correct order.
		 * If one of the file does not have the slice number, files are returned in default order.
		 */
		[[nodiscard]] std::vector<std::filesystem::path> SortDicomSlices(const std::vector<std::filesystem::path>& paths) const;

		/*
		 * Initiliazes file type based on allocated bits tag in dicom file.
		 */
		void ResolveFileType();

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
		std::vector<glm::vec4> m_Data{};
	};
}