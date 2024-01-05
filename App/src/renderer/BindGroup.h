#pragma once
#include "webgpu/webgpu.h"
#include "UniformBuffer.h"
#include "StorageBuffer.h"
#include "Texture.h"
#include "Sampler.h"

#include <iostream>
#include <vector>
#include <string>

namespace med
{
	class BindGroup
	{
	private:
		static int s_NumberOfBindGroups;
		static int s_CurrentBind;
	public:
		//TODO: Pass context ?
		BindGroup();
		~BindGroup();
	public:
		/*
		 * This static method resets the counter of currently bound bind-groups.
		 * Should be called at the ending of the render-pass.
		 * The should is a must, when re-using different bind groups in different pipelines.
		 */
		static void ResetBindSlotsIndices();
	public:
		/*
		* Add uniform buffer to the bind group.
		* There are no default flags for visibility.
		* Bindings are assigned automatically on first come, first served basis.
		*/
		void AddBuffer(const UniformBuffer& ub, WGPUShaderStageFlags visibility);
		
		/*
		* Add storage buffer to the bind group.
		* There are no default flags for visibility.
		* Bindings are assigned automatically on first come, first served basis.
		*/
		void AddBuffer(const StorageBuffer& sb, WGPUShaderStageFlags visibility);

		/*
		* Add Texture to the bind group.
		* There are no default flags for visibility.
		* Bindings are assigned automatically on first come, first served basis.
		*/
		void AddTexture(const Texture& tex, WGPUShaderStageFlags visibility, WGPUTextureSampleType sampleType);


		/*
		* Add Sampler to the bind group.
		* Visibility is set to Fragment and cannot be changed, same for sampler type if needed to extend this.
		* Bindings are assigned automatically on first come, first served basis.
		*/
		void AddSampler(const Sampler& s);

		/*
		* Prints to stdout binding names and their binding values.
		*/
		void ListBindings() const;

		/*
		* Creates necessary objects for pipeline and finalizes the creation of bind group on GPU.
		* Must be called only once, and after it's called bind group is immutable.
		*/
		void FinalizeBindGroup(const WGPUDevice& device);

		/*
		* Bind bind-group for this render pass.
		*/
		void Bind(const WGPURenderPassEncoder& pass) const;

		WGPUBindGroupLayout GetLayout() const;

		void SetBindGroupName(std::string&& name);

		bool IsInitialized() const;

	private:

		/*
		* Sets default values, that are then configured based on the entry.
		*/
		void SetDefaultBindGroupLayoutEntry(WGPUBindGroupLayoutEntry& bindingLayout);
		
		/*
		* Helper to avoid repeating code when we are creating
		* BingGroupLayoutEntry.
		*/
		void SetupEntry(WGPUBuffer buffer, WGPUBufferBindingType type, uint64_t size, WGPUShaderStageFlags visibility, std::string&& name);
		
		/*
		* Updates Layout description, must be called after every add operation.
		* Updates description object for pipeline (obj. for pipeline is created from it).
		*/
		void UpdateBindGroupLayoutDescriptor();

	private:
		std::vector<WGPUBindGroupLayoutEntry> m_BindGroupLayoutEntries{};
		// Created with data from ^, store this ?
		WGPUBindGroupLayoutDescriptor m_BindGroupLayoutDesc{};
		// Created from ^, this object is passed to the pipeline
		WGPUBindGroupLayout m_Layout;

		// Used to connect binding with buffers
		std::vector<WGPUBindGroupEntry> m_BindGroupEntries;

		// Selfexpl. uses ^ and WGPUBindGroupLayout, store this ?
		WGPUBindGroupDescriptor m_BindGroupDescriptor{};
		// Created from ^
		WGPUBindGroup m_BindGroupObject;

		bool m_IsInitialized = false;
		int m_BindingsCount = 0;
		std::string m_Name;
		std::vector<std::string> m_BindingsName;
	};
}