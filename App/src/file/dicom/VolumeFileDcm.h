#pragma once

#include "../VolumeFile.h"
#include "DicomParams.h"
#include "IDicomFile.h"
#include "glm/glm.hpp"

#include <filesystem>
#include <vector>
#include <map>

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
		
		/*
		* @brief Method deduced from the upper two methods (not defined in the dicom spec.)
		* Also returns the Z value for the pixel.
		*/
		glm::vec3 RCSToVoxelTransform(glm::vec3 coord) const;

		/* IDicom Interface */
		DicomBaseParams GetBaseParams() const override;
		
		DicomModality GetModality() const override;
		
		bool CompareFrameOfReference(const IDicomFile& other) const override;
		
		DicomVolumeParams GetVolumeParams() const;

		/*
		*  ================ These are contour related things, in the future ContourDcm could extend this VolumeFileDcm class ===================
		*/

		/*
		* @brief Sets the variable that holds slice number. For instance for first contour the contour
		* is present only in images (slices) 25, 26, 27 then this array looks like this [ [25, 26, 27] ]
		* May speed up processing on images as some of them can be skipped.
		*/
		void SetContourSliceNumbers(std::vector<std::vector<int>> sliceNumbers);

	private:

		/*
		* @brief Helper, initializes the dicom's Pixel <--> RCS matrices
		*/
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
		
		// Dicom's BitsStored leaves room for additional information or to accommodate data from systems that may acquire data at higher bit depths in the future
		// therefore it's overshoot to 16bits, but as we want to use the TF as much as possible and be compatible with as many files as possible we use closest bit depth
		// if we would have used the max value of a file to use TF to the max, we would have to do linear mappings to the interval of this to use this file's TF
		// now we just divide by 2^m_CustomBitWidth (but we also know that CT values have some meaning, this is morelikely to happen for MRI)

		DicomVolumeParams m_Params;

		// ContourDcm class (see FillContour docstring)
		// Key: Slice number, Valu: number of objects in the slice
		std::vector<std::map<int, int>> m_CtrSliceNum{};
	};
}