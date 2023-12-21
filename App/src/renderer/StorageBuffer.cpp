#include "StorageBuffer.h"

namespace med
{
	void StorageBuffer::UpdateBuffer(const WGPUQueue& queue, uint64_t offset, const void* data, size_t size) const
	{
		wgpuQueueWriteBuffer(queue, m_Buffer, offset, data, size);
	}
	
	WGPUBuffer StorageBuffer::GetBufferPtr() const
	{
		return m_Buffer;
	}

	std::uint32_t StorageBuffer::GetSize() const
	{
		return m_Size;
	}

	std::string StorageBuffer::GetName() const
	{
		return m_Name;
	}
}