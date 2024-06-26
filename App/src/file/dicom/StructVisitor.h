#pragma once

#include "dcm/defs.h"
#include "dcm/data_element.h"
#include "dcm/data_sequence.h"
#include "dcm/data_set.h"
#include "dcm/dicom_file.h"
#include "dcm/visitor.h"
#include "Base/Base.h"
#include "DicomParams.h"
#include "DicomParseUtil.h"

#include <vector>

namespace med
{
	// Maybe find a way to retrieve the dataelemetns and directly parse them without traversing it this way
	class StructVisitor : public dcm::Visitor
	{
	public:

		void VisitDataElement(const dcm::DataElement* dataElement) override
		{
			const auto& tag = dataElement->tag();
			const auto& vr = dataElement->vr();

			if (tag == kContourData)
			{
				std::string data{};
				dataElement->GetString(&data);
				auto indexOfContour = ContourData.size() - 1;
				auto contourNumber = ContourData[indexOfContour].size() - 1;
				ContourData[indexOfContour].push_back(ParseContours(data));
			}
			else if (tag == kDisplayColor)
			{
				constexpr float factor = 1/255.0f;
				std::string data;
				dataElement->GetString(&data);
				auto d = ParseStringToNumArr<int, 3>(data);
				Params.DisplayColors.emplace_back(d[0] * factor, d[1] * factor, d[2] * factor);
			}
			else if (tag == kROINumber)
			{
				std::string data;
				dataElement->GetString(&data);
				Params.StructureSetROISequence.back().Number = ParseStringToNumArr<int, 1>(data)[0];
			}
			else if (tag == kROIName)
			{
				dataElement->GetString(&Params.StructureSetROISequence.back().Name);
			}
			else if (tag == kROIGenerationAlgorithm)
			{
				dataElement->GetString(&Params.StructureSetROISequence.back().AlgorithmType);
			}
			else if (tag == kFrameOfReference)
			{
				dataElement->GetString(&Params.FrameOfReference);
			}
			else if (tag == kStructureSetName)
			{
				dataElement->GetString(&Params.Name);
			}
			else if (tag == kStructureSetLabel)
			{
				dataElement->GetString(&Params.Label);
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
					if (dataSequence->tag() == kStructureSetROISequence)
					{
						Params.StructureSetROISequence.emplace_back();
					}
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
		std::vector<std::vector<std::vector<float>>> ContourData{};
	private:
		const dcm::Tag kFrameOfReference = 0x00200052;
		const dcm::Tag kStructureSetName = 0x30060004;
		const dcm::Tag kStructureSetLabel = 0x30060002;


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