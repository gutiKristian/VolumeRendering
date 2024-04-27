#pragma once

#include "../renderer/Texture.h"

#include <string>
#include <memory>

namespace med
{
	class TransferFunction
	{
	public:
		virtual void Render() = 0;
		virtual void UpdateTexture() = 0;
		virtual std::string GetType() const = 0;
		virtual void Save(const std::string& name) = 0;
		virtual void Load(const std::string& name) = 0;
	protected:
		void ResolveResolution(int resolution);

	public:
		std::shared_ptr<Texture> GetTexture() const;
		int GetMaxTextureResolution() const;
		int GetTextureResolution() const;

	protected:
		std::shared_ptr<Texture> p_Texture = nullptr;
		int m_TextureResolution = 0;
		bool m_ShouldUpdate = false;
	};
}