#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "../renderer/Texture.h"

namespace med
{
	class ColorTF
	{
	public:
		explicit ColorTF(int desiredTfResolution);

		std::shared_ptr<Texture> GetTexture() const;
		void Render();
		void UpdateTexture();
		void Save(const std::string& name);
		void Load(const std::string& name);
	private:
		void UpdateColors(int cpId);
	private:
		int m_DataDepth;
		int m_ClickedCpId = -1;
		std::shared_ptr<Texture> p_Texture = nullptr;
		std::vector<glm::vec4> m_Colors{};
		// Control points positions and colors
		std::vector<glm::dvec2> m_ControlPos{};
		std::vector<glm::vec4> m_ControlCol{};
		bool m_ShouldUpdate = false;
		bool m_HasClicked = false;
	};
}