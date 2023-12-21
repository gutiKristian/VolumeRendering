#include "IndexBuffer.h"

namespace med
{
	IndexBuffer::IndexBuffer(WGPUBuffer buffer, std::uint32_t size, std::uint32_t elemCount, const std::string& name) noexcept :
		m_Buffer(buffer), m_Size(size), m_ElemCount(elemCount), m_Name(name)
	{
	}

	/*
	 * Create index buffer.
	 * Param size is in bytes.
	 */
	std::shared_ptr<IndexBuffer> IndexBuffer::CreateFromData(const WGPUDevice& device, const WGPUQueue& queue, const void* dataPtr,
		std::uint32_t size, std::uint32_t elemCount, std::string&& name)
	{
		WGPUBufferDescriptor desc{};
		desc.nextInChain = nullptr;
		desc.size = size;
		desc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Index;
		WGPUBuffer buffer =  wgpuDeviceCreateBuffer(device, &desc);
		wgpuQueueWriteBuffer(queue, buffer, 0, dataPtr, size);
		return std::make_shared<IndexBuffer>(buffer, size, elemCount, name);
	}

	WGPUBuffer IndexBuffer::GetBufferPtr() const
	{
		return m_Buffer;
	}

	/*
	* Returns size in bytes
	*/ 
	std::uint32_t IndexBuffer::GetSize() const
	{
		return m_Size;
	}

	std::uint32_t IndexBuffer::GetCount() const
	{
		return m_ElemCount;
	}

	std::string IndexBuffer::GetName() const
	{
		return m_Name;
	}
}