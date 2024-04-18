#pragma once

#include "../VolumeFile.h"
#include "DicomParams.h"
#include "IDicomFile.h"
#include "glm/glm.hpp"

#include <filesystem>
#include <vector>

namespace med
{
	class VolumeFileDcm : public IDicomFile, public VolumeFile
	{
	public:
		VolumeFileDcm(std::filesystem::path path, std::tuple<std::uint16_t, std::uint16_t, std::uint16_t> size,
			FileDataType type, DicomVolumeParams params, std::vector<glm::vec4>& data);
	public:
		/*
		* @brief Using Image Orientation (Patient) Attribute compares if these images face the same direction
		* and might be overlayed. If there's struct file made from this (e.g. mask) then that should be rotated as well.
		* @param other: VolumeFileDcm dataset
		* @return: true if they face same dir., false otherwise
		*/
		bool CompareOrientation(const VolumeFileDcm& other) const;

		/*
		* @brief Two methods to transform coordinates from pixel data (image space) to refernce coordinate system (RCS) and vice versa.
		* RCS is the coordinate system of the machine that generated the data.
		*/
		glm::vec3 PixelToRCSTransform(glm::vec2 coord) const;
		
		glm::vec2 RCSToPixelTransform(glm::vec3 coord) const;
		
		/* IDicom Interface */
		DicomBaseParams GetBaseParams() const override;
		
		DicomModality GetModality() const override;
		
		bool CompareFrameOfReference(const IDicomFile& other) const override;
		
		DicomVolumeParams GetVolumeParams() const;

	private:
		void InitializeTransformMatrices();
		
		/*
		 * @brief Calculate the main axis of the volume based on the DICOM image orientation.
		 *
		 * This function calculates the main axis of the volume represented by the DICOM image
		 * based on the orientation information stored in the ImageOrientationPatient tag.
		 * The resulting main axis is stored in the m_Params.MainAxis member variable.
		 * @note This function assumes that the ImageOrientationPatient tag has been properly set.
		 */
		void CalcMainAxis();

	private:
		glm::mat4x4 m_PixelToRCS{ 1.0f };
		glm::mat4x4 m_RCSToPixel{ 1.0f };
		DicomVolumeParams m_Params;
	};
}