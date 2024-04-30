#pragma once

#include "../renderer/Texture.h"

#include <glm/glm.hpp>
#include <string>
#include <memory>
#include <vector>

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
		virtual void UpdateYAxis(int cpId) = 0;
	// Implemented
	protected:
		void ResolveResolution(int resolution);
	public:
		std::shared_ptr<Texture> GetTexture() const;
		int GetMaxTextureResolution() const;
		int GetTextureResolution() const;
		int AddControlPoint(double mouseX, double mouseY, bool updateOnAdd = true);
	protected:
		std::shared_ptr<Texture> p_Texture = nullptr;
		std::vector<glm::dvec2> m_ControlPoints{};
		int m_TextureResolution = 0;
		bool m_ShouldUpdate = false;
	};
}