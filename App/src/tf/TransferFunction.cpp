#include "TransferFunction.h"
#include "Base/GraphicsContext.h"
#include "Base/Base.h"

#include <cassert>

namespace med
{
	void TransferFunction::ResolveResolution(int resolution)
	{
		m_TextureResolution = resolution;
		int maxTex1Dsize = GetMaxTextureResolution();

		if (maxTex1Dsize < m_TextureResolution)
		{
			LOG_WARN("Max data value is greater than max texture 1D size, clamping to max texture size");
			m_TextureResolution = maxTex1Dsize;
		}
		else if (m_TextureResolution <= 0)
		{
			LOG_INFO("Texture resolution will be set to maximum possible range.");
			std::string s = "Texture resolution set to: " + std::to_string(maxTex1Dsize);
			LOG_INFO(s.c_str());
			m_TextureResolution = maxTex1Dsize;
		}
	}

	std::shared_ptr<Texture> TransferFunction::GetTexture() const
	{
		assert(p_Texture != nullptr && "Texture is not initialized");
		return p_Texture;
	}

	int TransferFunction::GetMaxTextureResolution() const
	{
		return base::GraphicsContext::GetLimits().maxTextureDimension1D;
	}

	int TransferFunction::GetTextureResolution() const
	{
		return m_TextureResolution;
	}
}