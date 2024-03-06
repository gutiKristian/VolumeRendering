#pragma once

#include "../renderer/Texture.h"
#include "../file/VolumeFile.h"

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <string>

namespace med
{
	// In the future create a base class for transfer functions, to avoid code duplication
	class OpacityTF
	{
	public:
		explicit OpacityTF(int desiredTfResolution);

		std::shared_ptr<Texture> GetTexture() const;
		
		/*
		* @brief Render the transfer function
		*/
		void Render();

		/*
		* @brief Update data on the gpu
		*/
		void UpdateTexture();
		
		/*
		* When called, histogram overlay is plotted.
		*/
		void ActivateHistogram(const VolumeFile& file);

		/*
		* Save and load the transfer function to a file,
		* still avoiding the use of virtual functions (as render is called every frame)
		* maybe in the future refactor with some Saveable interface (same for ColorTF)
		*/
		void Save(const std::string& name);
		void Load(const std::string& name);
	private:
		/*
		* @brief Recalculate the interval between control points
		*/
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