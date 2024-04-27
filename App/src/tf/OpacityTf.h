#pragma once

#include "TransferFunction.h"
#include "../renderer/Texture.h"
#include "../file/VolumeFile.h"

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <string>

namespace med
{
	// In the future create a base class for transfer functions, to avoid code duplication
	class OpacityTF : public TransferFunction
	{
	public:
		explicit OpacityTF(int desiredTfResolution);

		/*
		* @brief Render the transfer function
		*/
		void Render() override;

		/*
		* @brief Update data on the gpu
		*/
		void UpdateTexture() override;
		
		/*
		* When called, histogram overlay is plotted.
		*/
		void ActivateHistogram(const VolumeFile& file);

		std::string GetType() const override;

		/*
		* Save and load the transfer function to a file,
		* still avoiding the use of virtual functions (as render is called every frame)
		* maybe in the future refactor with some Saveable interface (same for ColorTF)
		*/
		void Save(const std::string& name) override;
		void Load(const std::string& name) override;
	private:
		/*
		* @brief Recalculate the interval between control points
		*/
		void UpdateYPoints(int controlPointIndex);
	private:
		std::vector<float> m_XPoints{};
		std::vector<float> m_YPoints{};
		std::vector<float> m_Histogram{};
		std::vector<glm::dvec2> m_ControlPoints{};

	};
}