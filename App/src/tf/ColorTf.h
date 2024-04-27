#pragma once

#include "TransferFunction.h"
#include "../renderer/Texture.h"

#include <vector>
#include <glm/glm.hpp>


namespace med
{
	class ColorTF : public TransferFunction
	{
	public:
		explicit ColorTF(int desiredTfResolution);

		void Render() override;
		std::string GetType() const override;
		void UpdateTexture() override;
		void Save(const std::string& name) override;
		void Load(const std::string& name) override;
	private:
		void UpdateColors(int cpId);
	private:
		int m_TextureResolution;
		int m_ClickedCpId = -1;
		std::vector<glm::vec4> m_Colors{};
		// Control points positions and colors
		std::vector<glm::dvec2> m_ControlPos{};
		std::vector<glm::vec4> m_ControlCol{};
		bool m_HasClicked = false;
	};
}