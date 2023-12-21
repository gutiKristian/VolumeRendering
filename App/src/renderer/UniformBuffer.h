#pragma once
#include "webgpu/webgpu.h"
#include <memory>
#include <string>

namespace med
{
	class UniformBuffer
	{
	public:
		UniformBuffer(WGPUBuffer buffer, std::uint32_t size, const std::string& name) noexcept;
		~UniformBuffer();
	public:
		/*
		* Creates new Uniform Buffer from provided description.
		* Default flags are: WGPUBufferUsage_CopyDst, WGPUBufferUsage_Uniform; [Additional might be provided]
		* Size is in bytes.
		* If you want to map the buffer at the creation set the mapped attrib.
		*/
		static std::shared_ptr<UniformBuffer> CreateFromData(const WGPUDevice& device, const WGPUQueue& queue, const void* dataPtr, std::uint32_t size, WGPUBufferUsageFlags additionalFlags = 0,
			bool mapped = false, std::string&& name = "UniformBuffer");

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

		std::string m_Name = "UniformBuffer";
		std::uint32_t m_Size = 0;

		// Used in WGPUBindGroupLayoutEntry construction (need the size)
		WGPUBufferDescriptor m_BufferDesc{};
		// Used for updating
		WGPUBuffer m_Buffer;

	};
}