#pragma once
#include "webgpu//webgpu.h"
#include <string>

namespace med
{
	class StorageBuffer
	{
	public:
		/*
		* Write inside uniform buffer and update its values.
		* Can be also used when you want to store multiple objects inside one uniform,
		* just make sure you preallocate enough memory.
		*/
		void UpdateBuffer(const WGPUQueue& queue, uint64_t offset, const void* data, size_t size) const;

		WGPUBuffer GetBufferPtr() const;

		std::uint32_t GetSize() const;

		std::string GetName() const;

	private:

		std::string m_Name = "StorageBuffer";
		std::uint32_t m_Size = 0;

		// Used in WGPUBindGroupLayoutEntry construction (need the size)
		WGPUBufferDescriptor m_BufferDesc{};
		// Used for updating
		WGPUBuffer m_Buffer;
	};
}