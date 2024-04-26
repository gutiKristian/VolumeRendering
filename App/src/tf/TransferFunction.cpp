#include "TransferFunction.h"

#include <cassert>

namespace med
{
	std::shared_ptr<Texture> TransferFunction::GetTexture() const
	{
		assert(p_Texture != nullptr && "Texture is not initialized");
		return p_Texture;
	}
}