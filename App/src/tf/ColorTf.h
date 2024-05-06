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
		/*
		* @brief Create TF controlling the color.
		* @param desiredTfResolution: Resultion of the texture for the TF representation, 0 means use MAXIMAL range.
		*/
		explicit ColorTF(int desiredTfResolution);

		void Render() override;
		std::string GetType() const override;
		void UpdateTexture() override;
		bool Save(const std::string& name) override;
		void Load(const std::string& name) override;
	private:
		void UpdateYAxis(int cpId) override;
	private:
		int m_ClickedCpId = -1;
		std::vector<glm::vec4> m_Colors{};
		std::vector<glm::vec4> m_ControlCol{};
		bool m_HasClicked = false;
	};
}