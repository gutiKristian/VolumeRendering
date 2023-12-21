#include "Texture.h"

#include <cassert>

namespace med
{

	Texture::Texture(WGPUTexture texture, WGPUTextureView textureView, WGPUTextureViewDescriptor viewDesc, std::string&& name) noexcept :
		m_Tex(texture), m_TexView(textureView), m_ViewDesc(viewDesc), m_Name(name)
	{
	}

	Texture::~Texture() noexcept
	{
		assert(m_Tex != nullptr && "Texture was already destroyed");
		wgpuTextureDestroy(m_Tex);
		wgpuTextureRelease(m_Tex);
	}

	std::shared_ptr<Texture> Texture::CreateFromData(const WGPUDevice& device, const WGPUQueue& queue, const void * dataPtr, WGPUTextureDimension dimension,
	                                                 std::tuple<std::uint16_t, std::uint16_t, std::uint16_t> size, WGPUTextureFormat format, WGPUTextureUsageFlags flags, std::uint32_t bytesPerElement, std::string&& name)
	{
		WGPUTextureDescriptor texDesc{};

		auto [x, y, z] = size;
		texDesc.dimension = dimension;
		texDesc.size = { x, y, z };
		texDesc.format = format;
		texDesc.usage = flags;

		// Default values, will implement setters
		texDesc.nextInChain = nullptr;
		texDesc.sampleCount = 1;
		texDesc.mipLevelCount = 1; // 1 deactivates this feature
		texDesc.viewFormatCount = 0;
		texDesc.viewFormats = nullptr;

		WGPUTexture texture = wgpuDeviceCreateTexture(device, &texDesc);


		WGPUTextureViewDescriptor textureViewDesc{};
		textureViewDesc.aspect = WGPUTextureAspect_All;
		textureViewDesc.baseArrayLayer = 0;
		textureViewDesc.arrayLayerCount = 1;
		textureViewDesc.baseMipLevel = 0;
		textureViewDesc.mipLevelCount = 1;
		textureViewDesc.dimension = WGPUTextureViewDimension_3D;
		textureViewDesc.format = format;


		WGPUTextureDataLayout srcTexLayout{};

		auto [width, height, depth] = size;
		srcTexLayout.nextInChain = nullptr;
		srcTexLayout.bytesPerRow = bytesPerElement * width;
		srcTexLayout.rowsPerImage = height;
		srcTexLayout.offset = 0;

		WGPUImageCopyTexture destination{};

		destination.mipLevel = 0; // just for now
		destination.origin = { 0, 0, 0 }; // offset
		destination.aspect = WGPUTextureAspect_All;
		destination.texture = texture;

		wgpuQueueWriteTexture(queue, &destination, dataPtr, (x * y * z * bytesPerElement), &srcTexLayout, &texDesc.size);

		WGPUTextureView textureView = wgpuTextureCreateView(texture, &textureViewDesc);

		return make_shared<Texture>(texture, textureView, textureViewDesc, std::move(name));
	}

	std::shared_ptr<Texture> Texture::CreateRenderAttachment(uint32_t width, uint32_t height, WGPUTextureUsageFlags flags, std::string&& name)
	{

		const auto device = base::GraphicsContext::GetDevice();

		WGPUTextureDescriptor texDesc{};

		texDesc.dimension = WGPUTextureDimension_2D;
		texDesc.size = { width, height, 1};
		texDesc.format = base::GraphicsContext::GetDefaultTextureFormat();
		texDesc.usage = WGPUTextureUsage_RenderAttachment | flags;

		// Default values, will implement setters
		texDesc.nextInChain = nullptr;
		// Multisampling -- maybe in the future put it as param?
		texDesc.sampleCount = 1;

		// 1 deactivates this feature
		texDesc.mipLevelCount = 1;
		texDesc.viewFormatCount = 0;
		texDesc.viewFormats = nullptr;

		WGPUTexture texture = wgpuDeviceCreateTexture(device, &texDesc);

		WGPUTextureViewDescriptor textureViewDesc{};
		textureViewDesc.aspect = WGPUTextureAspect_All;
		textureViewDesc.baseArrayLayer = 0;
		textureViewDesc.arrayLayerCount = 1;
		textureViewDesc.baseMipLevel = 0;
		textureViewDesc.mipLevelCount = 1;
		textureViewDesc.dimension = WGPUTextureViewDimension_2D;
		textureViewDesc.format = base::GraphicsContext::GetDefaultTextureFormat();

		WGPUTextureView textureView = wgpuTextureCreateView(texture, &textureViewDesc);

		return make_shared<Texture>(texture, textureView, textureViewDesc, std::move(name));
	}

	WGPUTextureViewDescriptor Texture::GetViewDescriptor() const
	{
		return m_ViewDesc;
	}

	WGPUTextureView Texture::GetTextureView() const
	{
		return m_TexView;
	}

	std::string Texture::GetName() const
	{
		return m_Name;
	}

}