#pragma once

#include "../renderer/Texture.h"
#include "../file/VolumeFile.h"

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <string>

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
		void Save(const std::string& name);
		void Load(const std::string& name);
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