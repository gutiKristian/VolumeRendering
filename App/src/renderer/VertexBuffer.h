#pragma once
#include "webgpu/webgpu.h"
#include <memory>
#include <cstdint>
#include <vector>
#include <string>

namespace med
{

	/*
	*  Working with VertexBuffer remainder
	* 1.When creating vertex buffer we specify it's size and slot/shader location and the data its self.
	*	This is updated onto the GPU device. This is just a first part.
	* 2.Now we have to define the vertex attributes that tells us how to interpret this data on the GPU.
	* 3.After the attributes are specified, class automatically calculates the layout (how to traverse the array)
	*/
	class VertexBuffer
	{
	public:

		/*
		* Note:
		*	- size is in bytes
		*	- slot represents shader location value
		*/
		VertexBuffer(WGPUBuffer buffer, std::uint32_t size, std::uint32_t slot, const std::string& name);
		~VertexBuffer();
	
		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer(VertexBuffer&& other) noexcept = delete;
		VertexBuffer& operator=(const VertexBuffer&) = delete;
		VertexBuffer& operator=(VertexBuffer&& other) noexcept = delete;
	
	public:
		/*
		* Creates new VertexBuffer from provided buffer.
		* Default flags are: WGPUBufferUsage_CopyDst, WGPUBufferUsage_Vertex; [Additional might be provided]
		* Size is in bytes and slot represents shader location
		* If you want to map the buffer at the creation set the mapped attrib.
		*/
		static std::shared_ptr<VertexBuffer> CreateFromData(const WGPUDevice& device, const WGPUQueue& queue, const void* dataPtr, std::uint32_t size, WGPUBufferUsageFlags additionalFlags = 0,
			bool mapped = false, std::string&& name = "VertexBuffer");
	
	public:

		/*
		* There are no attributes provided by default. Not setting attributes is an illformed VertexBuffer object.
		*/
		VertexBuffer& AddVertexAttribute(WGPUVertexAttribute attribute);

		/*
		* Default step mode is Vertex.
		*/
		VertexBuffer& SetStepMode(WGPUVertexStepMode stepMode);

		/*
		* Retrieve the layout of buffer. Used mainly when constructing pipeline. 
		*/
		WGPUVertexBufferLayout GetLayout() const;
		
		/*
		* Sets the slot to which this buffer is bound.
		*/
		void SetSlot(std::uint32_t slot);

		/*
		* Bind current Vertex buffer object to a provided render pass object, slot and size are stored within this object.
		*/
		void Bind(const WGPURenderPassEncoder& renderPass) const;

	private:

		void UpdateBufferLayout();

	private:				
		std::vector<WGPUVertexAttribute> m_VertexAttributes{};
		std::uint32_t m_Size;
		std::uint32_t m_Slot;
		std::string m_Name;
		WGPUVertexStepMode m_StepMode = WGPUVertexStepMode_Vertex;

		// Computed automatically
		WGPUBuffer m_BufferObj = nullptr;

		WGPUVertexBufferLayout m_Layout{
			.arrayStride = 0,
			.stepMode = WGPUVertexStepMode_Vertex,
			.attributeCount = static_cast<std::uint32_t>(m_VertexAttributes.size()),
			.attributes = m_VertexAttributes.data()
		};
	};
}