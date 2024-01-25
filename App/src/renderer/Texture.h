#pragma once
#include "webgpu/webgpu.h"
#include "Base/GraphicsContext.h"
#include <memory>
#include <string>

namespace med
{
	// VERY SPECIFIC TEXTURE CLASS, FOR NOW
	class Texture
	{
	public:
		Texture(WGPUTexture texture, WGPUTextureView textureView, WGPUTextureDescriptor texDesc, WGPUTextureViewDescriptor viewDesc,
			WGPUTextureDataLayout layout, WGPUImageCopyTexture destination, std::string&& name) noexcept;

		Texture(WGPUTexture texture, WGPUTextureView textureView, WGPUTextureDescriptor texDesc, WGPUTextureViewDescriptor viewDesc, std::string&& name) noexcept;

		~Texture() noexcept;
		
		/*
		* Creates texture from data. This texture can be updated aswell.
		*/
		static std::shared_ptr<Texture> CreateFromData(const WGPUDevice& device, const WGPUQueue& queue, const void* dataPtr, WGPUTextureDimension dimension,
			std::tuple<std::uint16_t, std::uint16_t, std::uint16_t> size, WGPUTextureFormat format, WGPUTextureUsageFlags flags, std::uint32_t bytesPerElement, std::string&& name = "Texture");
		/*
		 * Create texture as render attachment.
		 * Default flags: RenderAttachment
		 * TODO: context holding necessary information (width, height), assign them default values
		 */
		static std::shared_ptr<Texture> CreateRenderAttachment(uint32_t width, uint32_t height, WGPUTextureUsageFlags flags,  std::string&& name);
	public:
		void UpdateTexture(const WGPUQueue& queue, const void* dataPtr);

	public:
		WGPUTextureViewDescriptor GetViewDescriptor() const;
		WGPUTextureView GetTextureView() const;
		std::string GetName() const;
	private:
		static WGPUTextureViewDimension ResolveView(WGPUTextureDimension dim);
	
	private:
		// Instances
		WGPUTexture m_Tex;
		WGPUTextureView m_TexView;
		WGPUTextureDescriptor m_TexDesc;
		WGPUTextureViewDescriptor m_ViewDesc;
		// Uploading
		WGPUTextureDataLayout m_SrcTexLayout{};
		WGPUImageCopyTexture m_Destination{};

		std::string m_Name;
	};
}