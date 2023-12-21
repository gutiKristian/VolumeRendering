#include "UniformBuffer.h"
#include "webgpu/webgpu.h"
#include <cassert>

namespace med
{
	UniformBuffer::UniformBuffer(WGPUBuffer buffer, std::uint32_t size, const std::string& name) noexcept : m_Buffer(buffer), m_Size(size), m_Name(name)
	{
	}

	UniformBuffer::~UniformBuffer()
	{
		assert(m_Buffer != nullptr && "Hmmm");
		wgpuBufferDestroy(m_Buffer);
		wgpuBufferRelease(m_Buffer);
	}

	std::shared_ptr<UniformBuffer> UniformBuffer::CreateFromData(const WGPUDevice& device, const WGPUQueue& queue, const void* dataPtr, std::uint32_t size, WGPUBufferUsageFlags additionalFlags, bool mapped, std::string&& name)
	{
		WGPUBufferDescriptor descriptor{};
		descriptor.nextInChain = nullptr;
		descriptor.size = size;
		descriptor.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform | additionalFlags;
		descriptor.mappedAtCreation = mapped;

		WGPUBuffer uniformBuffer = wgpuDeviceCreateBuffer(device, &descriptor);
		wgpuQueueWriteBuffer(queue, uniformBuffer, 0, dataPtr, descriptor.size);

		return std::make_shared<UniformBuffer>(uniformBuffer, descriptor.size, name);
	}
	
	void UniformBuffer::UpdateBuffer(const WGPUQueue& queue, uint64_t offset, const void* data, size_t size) const
	{
		wgpuQueueWriteBuffer(queue, m_Buffer, offset, data, size);
	}

	WGPUBuffer UniformBuffer::GetBufferPtr() const
	{
		return m_Buffer;
	}

	std::uint32_t UniformBuffer::GetSize() const
	{
		return m_Size;
	}
	std::string UniformBuffer::GetName() const
	{
		return m_Name;
	}
}