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
		* @brief Fills the selected contours. Operation is done on the existing vector.
		*/
		void FillContour();

		/*
		* @brief Sets the variable that holds slice number. For instance for first contour the contour
		* is present only in images (slices) 25, 26, 27 then this array looks like this [ [25, 26, 27] ]
		* Drastically speeds up the flood filling algorithm as we dont have to look for the intersections in every slice.
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


		// ====== Private functions related to the Contours =======
		
		/*
		* @brief Finds the starting point for the flood fill. Assumes that the contours are enclosed.
		* This is naive implementation and might not work on some cases such as contour inside contour.
		* This case is also pretty tricky as human interaction might be needed.
		* @param slice: Basically index of the image in the 3D dataset
		* @param objects: Number of objects (contours) in one image
		* @return None, when seed is found flood fill is performed. If more objects are expected search continues.
		*/
		void FindSeeds(int slice, int objects);
		
		/*
		* @brief Iterative flood fill algorithm.
		* @param slice: index of the image inside the 3D data
		* @param seeds: starting points
		* @return None, all done on the existing vector
		*/
		void FloodFill(int slice, glm::ivec2 seed);

	private:
		glm::mat4x4 m_PixelToRCS{ 1.0f };
		glm::mat4x4 m_RCSToPixel{ 1.0f };
		DicomVolumeParams m_Params;

		// ContourDcm class (see FillContour docstring)
		// Key: Slice number, Valu: number of objects in the slice
		std::vector<std::map<int, int>> m_CtrSliceNum{};
	};
}