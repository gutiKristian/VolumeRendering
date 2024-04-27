#pragma once
#include "FileDataType.h"

#include <filesystem>
#include <utility>
#include <vector>
#include <tuple>
#include <glm/glm.hpp>

#include "FileSystem.h"

namespace med
{
	/**
	 * @brief Class representing a volume file, this class is used to load and store volume data.
	 * This class is used to store the data in a contiguous memory, so it can be easily uploaded to the GPU.
	 * Can be templatized to store different types of data, but for now, this is sufficient.
	 */
	class VolumeFile
	{
	public:
		VolumeFile() = default;
		VolumeFile(std::filesystem::path path, std::tuple<std::uint16_t, std::uint16_t, std::uint16_t> size,
			FileDataType type, std::vector<glm::vec4>& data);
		VolumeFile(std::filesystem::path path, std::tuple<std::uint16_t, std::uint16_t, std::uint16_t> size,
			FileDataType type, std::vector<glm::vec4>& data, size_t maxNumber);

		virtual ~VolumeFile() = default;

	protected:

		void SetData(std::vector<glm::vec4>& src);
		
		void SetFileDataType(FileDataType type);

		void SetDataSize(std::tuple<std::uint16_t, std::uint16_t, std::uint16_t> size);

		void SetPath(std::filesystem::path path);

		void SetMaxNumber(size_t maxNumber);
	
		size_t GetMaxNumber(const std::vector<glm::vec4>& vec);
	
	public:
		/**
		 * @brief Computes gradient using finite differences.
		 * @param normToZeroOne if true, the gradient will be normalized to the range [0, 1]
		 */
		void PreComputeGradient(bool normToZeroOne=false);
		void PreComputeGradientSobel();
		
		/*
		* @brief Normalizes the density values to [0, 1] interval.
		* Gradient values are untouched if already pre-computed.
		*/
		void NormalizeData();

		/*
		* @brief Get size/dimension of the input data.
		*/
		[[nodiscard]] std::tuple<std::uint16_t, std::uint16_t, std::uint16_t> GetSize() const;

		/**
		* @brief Data type saved inside of this file, setting this attribute is implemented by the subclass.
		*/
		[[nodiscard]] FileDataType GetFileType() const;
		
		/**
		* @brief Pointer to the contiguous memory where the data is stored.
		*/
		[[nodiscard]] const void* GetVoidPtr() const;

		[[nodiscard]] const std::vector<glm::vec4>& GetVecReference() const;

		/*
		* @brief Maximum number within the dataset. Used in normalization.
		* Note for dicom it is: 1 << GetMaxUsedBitDepth().
		*/
		[[nodiscard]] size_t GetMaxNumber() const;

		[[nodiscard]] int GetMaxUsedBitDepth() const;

		/**
		 * @brief Maps 3D coordinate to the 1D data.
		 * @param x coordinate
		 * @param y coordinate
		 * @param z coordinate
		 * @return index within 1D array
		 */
		[[nodiscard]] int GetIndexFrom3D(int x, int y, int z) const;

		/**
		 * @brief Returns the density value at position (x, y,z)
		 * @param x coordinate
		 * @param y coordinate
		 * @param z coordinate
		 * @return density, if coords are outside of the volume returns vec4(0.0f)
		 */
		[[nodiscard]] glm::vec4 GetVoxelData(int x, int y, int z) const;

	protected:
		bool m_HasGradient = false;

		FileDataType m_FileDataType = FileDataType::Undefined;
		std::filesystem::path m_Path{};
		
		std::tuple<std::uint16_t, std::uint16_t, std::uint16_t> m_Size{ 0, 0, 0 };

		size_t m_MaxNumber = 0;
		int m_CustomBitWidth = 0;

		std::vector<glm::vec4> m_Data{};
		std::vector<float> m_DataSqueezed{};
		//TODO: Uploads to the gpu are 4x bigger rn, in the future use templates or when really needed use squeezed arr

		float m_SobelX[3][3][3] = {
			{ { -1, 0, 1 }, { -2, 0, 2 }, { -1, 0, 1 } },
			{ { -2, 0, 2 }, { -4, 0, 4 }, { -2, 0, 2 } },
			{ { -1, 0, 1 }, { -2, 0, 2 }, { -1, 0, 1 } }
		};
		float m_SobelY[3][3][3] = {
			{ { -1, -2, -1 }, { 0, 0, 0 }, { 1, 2, 1 } },
			{ { -2, -4, -2 }, { 0, 0, 0 }, { 2, 4, 2 } },
			{ { -1, -2, -1 }, { 0, 0, 0 }, { 1, 2, 1 } }
		};
		float m_SobelZ[3][3][3] = {
			{ { -1, -2, -1 }, { -2, -4, -2 }, { -1, -2, -1 } },
			{ { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
			{ { 1, 2, 1 }, { 2, 4, 2 }, { 1, 2, 1 } }
		};
	};
}