#pragma once

#include "../renderer/Texture.h"
#include "../file/VolumeFile.h"

#include <vector>
#include <memory>
#include <glm/glm.hpp>

namespace med
{
	class OpacityTF
	{
	public:
		explicit OpacityTF(int maxDataValue);

		std::shared_ptr<Texture> GetTexture() const;
		bool Render();
		void UpdateTexture();
		void ActivateHistogram(const VolumeFile& file);
	private:
		void UpdateYPoints(int controlPointIndex);
	private:
		bool m_ShouldUpdate = false;
		int m_DataDepth;
		std::vector<float> m_XPoints{};
		std::vector<float> m_YPoints{};
		std::vector<float> m_Histogram{};
		std::vector<glm::dvec2> m_ControlPoints{};
		std::shared_ptr<Texture> p_Texture = nullptr;
	};
}