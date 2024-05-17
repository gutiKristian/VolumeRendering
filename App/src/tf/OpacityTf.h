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
	class OpacityTF : public TransferFunction
	{
	public:
		/*
		* @brief Create TF controlling the opacity.
		* @param desiredTfResolution: Resultion of the texture for the TF representation, 0 means use MAXIMAL range.
		*/
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
		bool Save(const std::string& name) override;
		void Load(const std::string& name, TFLoadOption option = TFLoadOption::NONE) override;

		void ResetTF() override;

		/*
		* @brief Calibrates transfer function based on the provided mask. The mask is usually a filled contour.
		* @param mask: The mask which determines the areas where calibration is performed
		* @param data: File used to generate the histogram inside the areas
		* @param activeContours: As mask may have up to 4 contours, this param tells us which should be taken into account
		*/
		void CalibrateOnMask(std::shared_ptr<const VolumeFile> mask, std::shared_ptr<const VolumeFile> file, std::array<int, 4> activeContours);
	private:
	/*
	* @brief Recalculate the interval between control points
	*/
	void UpdateYAxis(int cpId) override;

	private:
		std::vector<float> m_XPoints{};
		std::vector<float> m_YPoints{};
		std::vector<float> m_Histogram{};
	};
}