#pragma once

#include <string>
#include <filesystem>
#include <cassert>
#include "FileDataType.h"

namespace med
{
	class VolumeFile
	{
	protected:
		using Size = std::tuple<std::uint16_t, std::uint16_t, std::uint16_t>;
	public:
		VolumeFile() = default;
		VolumeFile(const std::filesystem::path& path, bool isDir) : m_Path(path), m_IsDir(isDir) {}
		
	public:
		void setDataPtr(void* dataPointer)
		{
			assert(p_Data == nullptr);
			p_Data = std::shared_ptr<void>(dataPointer);
		}
		void setFileDataType(FileDataType type) { m_FileDataType = type; }
		void setSize(Size size) { m_Size = size; }
	public:
		/*
		* Get size/dimension of the input data.
		*/
		Size getSize() const { return m_Size; }

		/*
		* Data type saved inside of this file, setting this attribute is implemented by the subclass.
		*/
		FileDataType getFileType() const
		{
			return m_FileDataType;
		}

		/*
		* Gets the pointer to the data, this data is saved in
		* contiguous memory (even 3D), this pointer should be used to upload data into the GPU
		*/
		const void* getDataPtr() const
		{
			return p_Data.get();
		}

		const void* get4BPtr()
		{
			if (p_DataFloat == nullptr)
			{
				CalculateFloatData();
			}
			return p_DataFloat.get();
		}

	private:
		void CalculateFloatData()
		{
			auto [x,y,z] = m_Size;
			const auto size = static_cast<uint32_t>(x * y * z);

			float* fData = new float[x * y * z];
			const std::uint16_t* data = static_cast<std::uint16_t*>(p_Data.get());

			for (std::uint32_t i = 0; i < size; ++i)
			{
				fData[i] = static_cast<float>(data[i]) / 4095.0f;
			}
			p_DataFloat = std::shared_ptr<void>(fData);
		}

	protected:
		bool m_IsDir;
		std::shared_ptr<void> p_Data = nullptr;
		std::shared_ptr<void> p_DataFloat = nullptr;
		std::filesystem::path m_Path;
		FileDataType m_FileDataType = FileDataType::Undefined;
		Size m_Size{ 0, 0, 0 };
		
	};
}