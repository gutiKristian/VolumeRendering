#include "VertexBuffer.h"
#include <cassert>

namespace med
{

	static uint32_t GetFormatSize(WGPUVertexFormat format)
	{
		constexpr float floatVal = sizeof(float);

		switch (format)
		{
		case WGPUVertexFormat::WGPUVertexFormat_Force32:
			return 4;
		case WGPUVertexFormat::WGPUVertexFormat_Float32:
			return floatVal;
		case WGPUVertexFormat::WGPUVertexFormat_Float32x2:
			return floatVal * 2;
		case WGPUVertexFormat::WGPUVertexFormat_Float32x3:
			return floatVal * 3;
		case WGPUVertexFormat::WGPUVertexFormat_Float32x4:
			return floatVal * 4;
		}

		assert(false && "Unknown datatype");
		return 0;
	}

	VertexBuffer::VertexBuffer(WGPUBuffer buffer, std::uint32_t size, std::uint32_t slot, const std::string& name) : m_BufferObj(buffer), m_Size(size), m_Slot(slot), m_Name(name)
	{
	}

	VertexBuffer::~VertexBuffer()
	{
		assert(m_BufferObj != nullptr && "Vertex Buffer was never set!");
		wgpuBufferDestroy(m_BufferObj);
		wgpuBufferRelease(m_BufferObj);
	}

	std::shared_ptr<VertexBuffer> VertexBuffer::CreateFromData(const WGPUDevice& device, const WGPUQueue& queue, const void* dataPtr, std::uint32_t size, WGPUBufferUsageFlags additionalFlags, bool mapped, std::string&& name)
	{
		WGPUBufferDescriptor descriptor{};
		descriptor.nextInChain = nullptr;
		descriptor.size = size;
		descriptor.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex | additionalFlags;
		descriptor.mappedAtCreation = mapped;

		WGPUBuffer vertexBuffer = wgpuDeviceCreateBuffer(device, &descriptor);
		wgpuQueueWriteBuffer(queue, vertexBuffer, 0, dataPtr, descriptor.size);

		return std::make_shared<VertexBuffer>(vertexBuffer, descriptor.size, 0, name);
	}

	VertexBuffer& VertexBuffer::AddVertexAttribute(WGPUVertexAttribute attribute)
	{
		// In the future might help check the user(me) offset

		m_VertexAttributes.push_back(attribute);
		UpdateBufferLayout();
		return *this;
	}

	VertexBuffer& VertexBuffer::SetStepMode(WGPUVertexStepMode stepMode)
	{
		m_StepMode = stepMode;
		m_Layout.stepMode = stepMode;
		return *this;
	}

	WGPUVertexBufferLayout VertexBuffer::GetLayout() const
	{
		return m_Layout;
	}

	void VertexBuffer::SetSlot(std::uint32_t slot)
	{
		m_Slot = slot;
	}

	void VertexBuffer::Bind(const WGPURenderPassEncoder& renderPass) const
	{
		assert(m_VertexAttributes.size() != 0 && m_BufferObj != nullptr && "Illformed VertexArray!");
		wgpuRenderPassEncoderSetVertexBuffer(renderPass, m_Slot, m_BufferObj, 0, m_Size);
	}

	void VertexBuffer::UpdateBufferLayout()
	{
		m_Layout.arrayStride += GetFormatSize(m_VertexAttributes.back().format);
		m_Layout.stepMode = m_StepMode;
		m_Layout.attributeCount = m_VertexAttributes.size();
		m_Layout.attributes = m_VertexAttributes.data();
	}

}