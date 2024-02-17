#pragma once
#include "FileDataType.h"

#include <filesystem>
#include <utility>

#include "glm/glm.hpp"

namespace med
{
	class VolumeFile
	{
	public:
		VolumeFile() = default;
		VolumeFile(std::filesystem::path  path, bool isDir) : m_Path(std::move(path)), m_IsDir(isDir) {}
		
	public:
		void SetData(std::vector<glm::vec4>& src);

		void SetFileDataType(FileDataType type);

		void SetDataSize(std::tuple<std::uint16_t, std::uint16_t, std::uint16_t> size);

		void PreComputeGradient();

		/*
		* Get size/dimension of the input data.
		*/
		[[nodiscard]] std::tuple<std::uint16_t, std::uint16_t, std::uint16_t> GetSize() const;

		/*
		* Data type saved inside of this file, setting this attribute is implemented by the subclass.
		*/
		[[nodiscard]] FileDataType GetFileType() const;

		/*
		* Gets the pointer to the data, this data is saved in
		* contiguous memory (even 3D), this pointer should be used to upload data into the GPU
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

	protected:
		std::filesystem::path m_Path{};
		bool m_IsDir = false;
		bool m_HasGradient = false;
		FileDataType m_FileDataType = FileDataType::Undefined;
		std::tuple<std::uint16_t, std::uint16_t, std::uint16_t> m_Size{ 0, 0, 0 };
		std::vector<glm::vec4> m_Data{};
	};
}