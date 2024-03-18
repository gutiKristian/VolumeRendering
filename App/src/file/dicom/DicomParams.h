#pragma once

#include "dcm/defs.h"
#include "dcm/data_element.h"
#include "dcm/data_sequence.h"
#include "dcm/data_set.h"
#include "dcm/dicom_file.h"
#include "dcm/visitor.h"
#include "Base/Base.h"

#include <array>
#include <string>
#include <vector>

namespace med
{
	enum class DicomModality
	{
		UNKNOWN, CT, RTSTRUCT, RTDOSE, MR
	};

	struct DicomBaseParams
	{
		DicomBaseParams(DicomModality mod) : Modality(mod) {}
		
		DicomModality Modality{ DicomModality::UNKNOWN };
		std::string FrameOfReference{};
	};

	// CT, MR, RTDose, PET
	struct DicomVolumeParams : public DicomBaseParams
	{
		DicomVolumeParams() : DicomBaseParams(DicomModality::UNKNOWN) {}
		
		std::uint16_t X = 0;											// (0028,0011) Columns
		std::uint16_t Y = 0;											// (0028,0010) Rows
		std::uint16_t Z = 0;											// (0028,0008) Number of Frames
		std::uint16_t BitsStored = 0;									// (0028,0101) Bits Stored
		std::uint16_t  BitsAllocated = 0;								// (0028,0100) Bits Allocated
		std::int16_t NumberOfFrames = 0;								// (0028,0008) Number of Frames
		double SliceThickness = 0.0;									// (0018,0050) Slice Thickness
		std::array<double, 3> ImagePositionPatient{ 0.0 };				// (0020,0032) Image Position (Patient)
		std::array<double, 6> ImageOrientationPatient{ 0.0 };			// (0020,0037) Image Orientation (Patient)
		std::array<double, 2> PixelSpacing{ 0.0 };						// (0028,0030) Pixel Spacing
	};


	// RTStruct
	struct DicomStructParams : public DicomBaseParams
	{
		DicomStructParams() : DicomBaseParams(DicomModality::RTSTRUCT) {}
	private:
		
		// (3006,0020) Structure Set ROI Sequence, description of the ROI (Region of Interest)
		struct StructureSetROI
		{
			int Number = 0;												// (3006,0022) RT ROI Number
			std::string Name{};											// (3006,0026) RT ROI Name
			std::string AlgorithmType{};								// (3006,002A) ROI Generation Algorithm
		};
		
		// (3006,0039) ROI Contour Sequence, description of the contour
		struct ROIContour
		{
			std::array<int, 3> DisplayColor{ 0 } ;						// (3006, 002A) ROI Display Color

		};

	public:
		// User defined label and name for the whole structure set
		std::string Label{};											// (3006,0002) RT Structure Set Label
		std::string Name{};												// (3006,0004) RT Structure Set Name

		std::vector<StructureSetROI> StructureSetROISequence;			
		std::vector<ROIContour> ROIContourSequence;
	};

	class StructVisitor : public dcm::Visitor
	{
	public:

		void VisitDataElement(const dcm::DataElement* dataElement) override
		{
			const auto& tag = dataElement->tag();
			const auto& vr = dataElement->vr();

			if (tag == kContourData)
			{
				std::string data="";
				dataElement->GetString(&data);
				auto indexOfContour = ContourData.size() - 1;
				auto indexOfSlice = ContourData[indexOfContour].size() - 1;
				//ContourData[indexOfContour][indexOfSlice].push_back(std::stof(data));
			}

			if (tag == kROINumber)
			{
				std::string name;
				dataElement->GetString(&name);
				LOG_TRACE(name.c_str());
			}
			if (tag == kROIName)
			{
				std::string name;
				dataElement->GetString(&name);
				LOG_TRACE(name.c_str());
			}
		}

		void VisitDataSequence(const dcm::DataSequence* dataSequence) override
		{
			
			if (dataSequence->tag() == kROIContourSequence)
			{
				std::string s = "Found " + std::to_string(dataSequence->size()) + " ROI Contours.";
				LOG_TRACE(s.c_str());
				VisitROIContourSequence(dataSequence);
			}
			else
			{
				if (dataSequence->tag() == kContourSequence)
				{
					std::string str = "Contour Sequence found. Size: " + std::to_string(dataSequence->size());
					LOG_TRACE(str.c_str());
				}

				for (size_t i = 0; i < dataSequence->size(); ++i)
				{
					const auto& item = dataSequence->At(i);

					VisitDataSet(item.data_set);
				}
			}
			
		}

		void VisitROIContourSequence(const dcm::DataSequence* dataSequence)
		{
			std::string s = "Adding vector for contour number: ";
			for (size_t i = 0; i < dataSequence->size(); ++i)
			{
				LOG_TRACE((s + std::to_string(i)).c_str());
				ContourData.push_back({});
				const auto& item = dataSequence->At(i);
				VisitDataSet(item.data_set);
			}
		}

		void VisitDataSet(const dcm::DataSet* dataSet) override
		{
			for (size_t i = 0; i < dataSet->size(); ++i)
			{
				auto tag = dataSet->At(i)->tag();

				(*dataSet)[i]->Accept(*this);
			}
		}

		DicomStructParams Params;
		std::vector<std::vector<std::vector<float>>> ContourData;
	private:
		const dcm::Tag kStructureSetROISequence = 0x30060020;
		const dcm::Tag kROIContourSequence = 0x30060039;
		const dcm::Tag kDisplayColor = 0x3006002A;
		// Structure Set ROI Sequence
		const dcm::Tag kROIName = 0x30060026;
		const dcm::Tag kROINumber = 0x30060022;
		const dcm::Tag kROIGenerationAlgorithm = 0x30060036;
		const dcm::Tag kROIDescription = 0x30060028;
		const dcm::Tag kROISequence = 0x30060020;
		const dcm::Tag kContourData = 0x30060050;
		const dcm::Tag kContourSequence = 0x30060040;

	};

}