#pragma once

#include "../FileReader.h"
#include "VolumeFileDcm.h"
#include "DicomParams.h"

#include "dcm/dicom_file.h"

#include <filesystem>
#include <array>
#include <memory>

namespace med
{
	class DicomReader : public FileReader
	{
	public:
		/**
		 * @brief Reads the dicom file and stores the data in the VolumeFileDcm object
		 * @param name path to the file
		 * @param isDir if true, the path is a directory, otherwise it is a file
		 * @return VolumeFileDcm object with the data
		 */
		[[nodiscard]] std::unique_ptr<VolumeFileDcm> ReadFile(const std::filesystem::path& name, bool isDir);

	private:
		/**
		 * @brief Reads all necessary tags from the dicom file
		 * @param f opened dicom file
		 */
		void ReadDicomVariables(const dcm::DicomFile& f);
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
		 * @brief Sorts the dicom files by the Instance number tag, if one file is missing this tag, same vector is returned
		 * @param paths dicom files
		 * @return sorted paths in ascending order
		 */
		[[nodiscard]] std::vector<std::filesystem::path> SortDicomSlices(const std::vector<std::filesystem::path>& paths) const;

		/**
		 * @brief Initiliazes file type based on allocated bits tag in dicom file.
		 */
		void ResolveFileType();

		/**
		 * Move to FileReader during refactoring.
		 * @brief Calculates how many bits is enough to represent the array. Based on this TF is created
		 * since we want to cover every possible density. So when
		 * @return actual used bit depth
		 */
		/*[[nodiscard]] int GetMaxUsedBits() const;*/

		
        /**
         * @brief Parses a string of numbers separated by '\\' into an array of doubles.
         * 
         * The function expects the string to be in the format "1\\0\\0\\0\\1\\0". It also handles optional square brackets at the beginning and end of the string.
         * If the string contains more numbers than the size of the array, the excess numbers are ignored. If it contains less, the remaining elements of the array are set to 0.0.
         * If a number cannot be parsed, it is skipped and does not affect the array.
         * 
         * @tparam N The size of the array to be returned.
         * @param str The string to be parsed. This parameter is passed by reference and will be modified by the function.
         * @return An array of doubles parsed from the string.
         */
        template <size_t N>
        std::array<double, N> DS(std::string& str)
        {
            // the string is in this format "1\\0\\0\\0\\1\\0"
            std::array<double, N> numbers{ 0.0 };
           
            // then parse the string expected number of numbers is specified by N
            std::stringstream ss(str);
            std::string temp;
            size_t i = 0;
            while (std::getline(ss, temp, '\\') && i < N)
            {
                try
                {
                    numbers[i++] = std::stod(temp);
                }
                catch (std::exception& e)
                {
                    // do nothing, just skip the number (decide in the future)
                }
            }
            return numbers;
        };

	private:
		DicomParams m_Params;
		FileDataType m_FileDataType = FileDataType::Undefined;
		std::vector<glm::vec4> m_Data{};
	};
}