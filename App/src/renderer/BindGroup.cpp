#include "BindGroup.h"
#include <cassert>

namespace med
{
	int BindGroup::s_NumberOfBindGroups = 0;
	int BindGroup::s_CurrentBind = 0;

	BindGroup::BindGroup()
	{
		// Used in debug message
		m_Name = "[]";
		++s_NumberOfBindGroups;
	}

	BindGroup::~BindGroup()
	{
		--s_NumberOfBindGroups;
	}

	void BindGroup::ResetBindSlotsIndices()
	{
		s_CurrentBind = 0;
	}

	void BindGroup::AddBuffer(const UniformBuffer& ub, WGPUShaderStageFlags visibility)
	{
		assert(!m_IsInitialized && "The bind group was already initialized, call this before FinalizeBindGroup.");
		SetupEntry(ub.GetBufferPtr(), WGPUBufferBindingType_Uniform, ub.GetSize(), visibility, std::move(ub.GetName()));
	}

	void BindGroup::AddBuffer(const StorageBuffer& sb, WGPUShaderStageFlags visibility)
	{
		assert(!m_IsInitialized && "The bind group was already initialized, call this before FinalizeBindGroup.");
		SetupEntry(sb.GetBufferPtr(), WGPUBufferBindingType_Storage, sb.GetSize(), visibility, std::move(sb.GetName()));
	}

	void BindGroup::AddTexture(const Texture& tex, WGPUShaderStageFlags visibility, WGPUTextureSampleType sampleType)
	{
		assert(!m_IsInitialized && "The bind group was already initialized, call this before FinalizeBindGroup.");

		WGPUBindGroupLayoutEntry entry{};
		SetDefaultBindGroupLayoutEntry(entry);
		entry.nextInChain = nullptr;
		entry.binding = m_BindingsCount;
		entry.visibility = visibility;
		entry.texture.sampleType = sampleType;
		entry.texture.viewDimension = tex.GetViewDescriptor().dimension;

		m_BindGroupLayoutEntries.push_back(entry);


		WGPUBindGroupEntry bindGroupEntry{};
		bindGroupEntry.nextInChain = nullptr;
		bindGroupEntry.binding = m_BindingsCount;
		bindGroupEntry.textureView = tex.GetTextureView();
		
		m_BindGroupEntries.push_back(bindGroupEntry);

		UpdateBindGroupLayoutDescriptor();


		++m_BindingsCount;
	}

	void BindGroup::UpdateBindGroupLayoutDescriptor()
	{
		m_BindGroupLayoutDesc.entries = m_BindGroupLayoutEntries.data();
		m_BindGroupLayoutDesc.entryCount = static_cast<uint32_t>(m_BindGroupLayoutEntries.size());
	}

	void BindGroup::AddSampler(const Sampler& s)
	{
		WGPUBindGroupLayoutEntry entry{};
		SetDefaultBindGroupLayoutEntry(entry);
		entry.nextInChain = nullptr;
		entry.binding = m_BindingsCount;
		entry.visibility = WGPUShaderStage_Fragment;
		entry.sampler.type = WGPUSamplerBindingType_Filtering;

		m_BindGroupLayoutEntries.push_back(entry);


		WGPUBindGroupEntry bindGroupEntry{};
		bindGroupEntry.nextInChain = nullptr;
		bindGroupEntry.binding = m_BindingsCount;
		bindGroupEntry.sampler = s.GetSampler();

		m_BindGroupEntries.push_back(bindGroupEntry);

		UpdateBindGroupLayoutDescriptor();

		++m_BindingsCount;
	}

	void BindGroup::ListBindings() const
	{
		for (int i = 0; i < m_BindingsName.size(); ++i)
		{
			// group index depends on the binding order, therefore debug outputs name of the bindgroup
			std::cout << "@group(" << m_Name << ") @binding(" << i << ")" << " " << m_BindingsName[i] << "\n";
		}
	}

	void BindGroup::SetupEntry(WGPUBuffer buffer, WGPUBufferBindingType type, uint64_t size, WGPUShaderStageFlags visibility, std::string&& name)
	{
		WGPUBindGroupLayoutEntry entry{};
		SetDefaultBindGroupLayoutEntry(entry);
		entry.nextInChain = nullptr;
		entry.binding = m_BindingsCount;
		entry.visibility = visibility;
		entry.buffer.type = type;
		entry.buffer.minBindingSize = size;
		
		m_BindGroupLayoutEntries.push_back(entry);

		WGPUBindGroupEntry bindGroupEntry{};
		bindGroupEntry.nextInChain = nullptr;
		bindGroupEntry.binding = m_BindingsCount;
		bindGroupEntry.size = size;
		bindGroupEntry.offset = 0;
		bindGroupEntry.buffer = buffer;

		// Update collection of entries
		m_BindGroupEntries.push_back(bindGroupEntry);
		
		UpdateBindGroupLayoutDescriptor();

		m_BindingsName.push_back(name);
		++m_BindingsCount;
	}

	void BindGroup::Bind(const WGPURenderPassEncoder& pass) const
	{
		assert(m_IsInitialized && "Not yet initialized");
		wgpuRenderPassEncoderSetBindGroup(pass, s_CurrentBind, m_BindGroupObject, 0, nullptr);
		++s_CurrentBind;
	}

	WGPUBindGroupLayout BindGroup::GetLayout() const
	{
		assert(m_IsInitialized && "Not yet initialized");
		return m_Layout;
	}

	void BindGroup::FinalizeBindGroup(const WGPUDevice& device)
	{
		assert(!m_IsInitialized && "Object was already created!");

		// Create object for pipeline
		m_Layout = wgpuDeviceCreateBindGroupLayout(device, &m_BindGroupLayoutDesc);

		// Finalize bind group creation
		m_BindGroupDescriptor.label = m_Name.c_str();
		m_BindGroupDescriptor.nextInChain = nullptr;
		m_BindGroupDescriptor.entryCount = static_cast<uint32_t>(m_BindGroupEntries.size());
		m_BindGroupDescriptor.entries = m_BindGroupEntries.data();
		m_BindGroupDescriptor.layout = m_Layout;

		m_BindGroupObject = wgpuDeviceCreateBindGroup(device, &m_BindGroupDescriptor);
		m_IsInitialized = true;
	}

	void BindGroup::SetBindGroupName(std::string&& name)
	{
		assert(!m_IsInitialized && "The bind group was already initialized, call this before FinalizeBindGroup.");
		m_Name = name;
	}

	bool BindGroup::IsInitialized() const
	{
		return m_IsInitialized;
	}

	void BindGroup::SetDefaultBindGroupLayoutEntry(WGPUBindGroupLayoutEntry& bindingLayout)
	{
		bindingLayout.buffer.nextInChain = nullptr;
		bindingLayout.buffer.type = WGPUBufferBindingType_Undefined;
		bindingLayout.buffer.hasDynamicOffset = false;
		bindingLayout.sampler.nextInChain = nullptr;
		bindingLayout.sampler.type = WGPUSamplerBindingType_Undefined;
		bindingLayout.storageTexture.nextInChain = nullptr;
		bindingLayout.storageTexture.access = WGPUStorageTextureAccess_Undefined;
		bindingLayout.storageTexture.format = WGPUTextureFormat_Undefined;
		bindingLayout.storageTexture.viewDimension = WGPUTextureViewDimension_Undefined;
		bindingLayout.texture.nextInChain = nullptr;
		bindingLayout.texture.multisampled = false;
		bindingLayout.texture.sampleType = WGPUTextureSampleType_Undefined;
		bindingLayout.texture.viewDimension = WGPUTextureViewDimension_Undefined;
	}

}