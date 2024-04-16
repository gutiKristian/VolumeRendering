#pragma once

#include "../FileReader.h"
#include "VolumeFileDcm.h"
#include "StructureFileDcm.h"
#include "DicomParams.h"

#include "dcm/dicom_file.h"

#include <filesystem>
#include <array>
#include <memory>
#include <variant>

namespace med
{
	class DicomReader : public FileReader
	{
	public:
		/**
		 * @brief Reads the dicom file and stores the data in the VolumeFileDcm object, used for parsing image data
		 * @param name path to the file
		 * @return VolumeFileDcm object with the data
		 */
		[[nodiscard]] static std::shared_ptr<VolumeFileDcm> ReadVolumeFile(std::filesystem::path name);

		/**
		 * @brief Reads the dicom file and stores the data in the StructureFile object, is used for parsing contours
		 * @param name path to the file
		 * @return StructureFileDcm object with the data
		 */
		[[nodiscard]] static std::shared_ptr<StructureFileDcm> ReadStructFile(std::filesystem::path name);

		/*
		* @brief Retrieve modality of a file
		* @return DicomModality enum
		*/
		[[nodiscard]] static DicomModality CheckModality(const std::filesystem::path& name);
		
		/*
		* @brief Transfers string modaility to DicomModality enum, not case sensitive
		* @ return DicomModality enum
		*/
		[[nodiscard]] static DicomModality ResolveModality(std::string modality);
		
		/*
		* @brief Resolves enum DicomModality to string
		* @ return modality (uppercase string)
		*/
		[[nodiscard]] static std::string ResolveModality(DicomModality modality);

		/**
		 * @brief Sorts the dicom files by the Instance number tag, if one file is missing this tag, same vector is returned
		 * @param paths dicom files
		 * @return sorted paths in ascending order
		 */
		[[nodiscard]] static std::vector<std::filesystem::path> SortDicomSlices(const std::vector<std::filesystem::path>& paths);

		[[nodiscard]] static bool IsDicomFile(const std::filesystem::path& path);
	private:
		/**
		 * @brief Reads all necessary tags from the dicom file
		 * @param f opened dicom file
		 */
		void ReadDicomVolumeVariables(const dcm::DicomFile& f);
		/*
		* Based on detected type prepares memory for entire Volume.
		*/
		bool PreAllocateMemory(std::size_t frames, bool isDir);
		
		/**
		 * @brief Reads data into allocated memory (stored within the class to avoid copying)
		 * @param f opened dicom file
		 */
		void ReadData(const dcm::DicomFile& f);

		/**
		 * @brief Initiliazes file type based on allocated bits tag in dicom file.
		 */
		void ResolveFileType();

	private:
		DicomVolumeParams m_Params;
		FileDataType m_FileDataType = FileDataType::Undefined;
		std::vector<glm::vec4> m_Data{};
	};
}