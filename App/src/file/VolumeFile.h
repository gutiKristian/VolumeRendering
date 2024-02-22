#pragma once
#include "FileDataType.h"

#include <filesystem>
#include <utility>
#include <vector>
#include <tuple>

#include "glm/glm.hpp"

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
		explicit VolumeFile(std::filesystem::path path) : m_Path(std::move(path)) {}
		virtual ~VolumeFile() = default;

	public:

		void SetData(std::vector<glm::vec4>& src);
		
		void SetFileDataType(FileDataType type);

		void SetDataSize(std::tuple<std::uint16_t, std::uint16_t, std::uint16_t> size);

		void SetPath(std::filesystem::path path);

		/**
		 * @brief Computes gradient using finite differences.
		 */
		void PreComputeGradient();

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

	protected:
		/**
		 * @brief Maps 3D coordinate to the 1D data.
		 * @param x coordinate
		 * @param y coordinate
		 * @param z coordinate
		 * @return index within 1D array
		 */
		[[nodiscard]] int GetIndexFrom3D(int x, int y, int z) const;

		/**
		 * \brief Returns the density value at position (x, y,z)
		 * \param x coordinate
		 * \param y coordinate
		 * \param z coordinate
		 * \return density, if coords are outside of the volume returns vec4(0.0f)
		 */
		[[nodiscard]] glm::vec4 GetVoxelData(int x, int y, int z) const;

	protected:
		bool m_HasGradient = false;

		FileDataType m_FileDataType = FileDataType::Undefined;
		std::filesystem::path m_Path{};
		
		std::tuple<std::uint16_t, std::uint16_t, std::uint16_t> m_Size{ 0, 0, 0 };
		
		std::vector<glm::vec4> m_Data{};
		std::vector<float> m_DataSqueezed{};
		//TODO: Uploads to the gpu are 4x bigger rn, in the future use templates or when really needed use squeezed arr
	};
}