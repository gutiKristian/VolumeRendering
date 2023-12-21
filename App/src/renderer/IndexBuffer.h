#pragma once
#include "webgpu/webgpu.h"
#include <memory>
#include <string>

namespace med
{
	class IndexBuffer
	{
	public:
		IndexBuffer(WGPUBuffer buffer, std::uint32_t size, std::uint32_t elemCount, const std::string& name) noexcept;
	public:
		/*
		* Creates new Index Buffer from provided description.
		* Size is in bytes.
		*/
		static std::shared_ptr<IndexBuffer> CreateFromData(const WGPUDevice& device, const WGPUQueue& queue, const void* dataPtr,
			std::uint32_t size, std::uint32_t elemCount,std::string&& name = "IndexBuffer");

	public:
		
		WGPUBuffer GetBufferPtr() const;

		/*
		 * Returns size in bytes.
		 */
		std::uint32_t GetSize() const;

		/*
		 * Returns number of indices.
		 */
		std::uint32_t GetCount() const;

		std::string GetName() const;

	private:

		std::string m_Name = "IndexBuffer";
		std::uint32_t m_Size = 0;
		std::uint32_t m_ElemCount = 0;
		WGPUBufferDescriptor m_BufferDesc{};
		WGPUBuffer m_Buffer;

	};
}