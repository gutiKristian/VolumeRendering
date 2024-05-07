#include "OpacityTf.h"
#include "TfUtils.h"
#include "LinearInterpolation.h"
#include "Base/Base.h"
#include "Base/GraphicsContext.h"
#include "implot/implot.h"
#include "implot_internal.h"
#include "../file/FileSystem.h"

#include <cassert>
#include <fstream>
#include <sstream>

namespace med
{
	OpacityTF::OpacityTF(int desiredTfResolution)
	{
		ResolveResolution(desiredTfResolution);

		m_XPoints.resize(m_TextureResolution, 0.0f);
		m_YPoints.resize(m_TextureResolution, 0.0f);

		m_ControlPoints.emplace_back(0.0, 0.0);
		m_ControlPoints.emplace_back(m_TextureResolution - 1.0, 1.0);

		std::vector<float> result = LinearInterpolation::Generate<float>(0, m_TextureResolution - 1, 0.0f, 1.0f, 1);
		assert(result.size() == m_TextureResolution && "Size of generated TF does not match");

		// Fill for plotting
		for (size_t i = 0; i < m_TextureResolution; ++i)
		{
			m_XPoints[i] = i;
			m_YPoints[i] = result[i];
		}

		p_Texture = Texture::CreateFromData(base::GraphicsContext::GetDevice(), base::GraphicsContext::GetQueue(), m_YPoints.data(), WGPUTextureDimension_1D, {m_TextureResolution, 1, 1},
			WGPUTextureFormat_R32Float, WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst, sizeof(float), "Opacity TF");
	}

	// Maybe in the future refactor: HandleClick, HandleDrag
	void OpacityTF::Render()
	{
		if (ImPlot::BeginPlot("##optfplot"))
		{
			// This sets up axes 1
			ImPlot::SetupAxes("Voxel value", "Alpha", 0, ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit);

			// Setup limits, X: 0-4095 (data resolution -- bits per pixel), Y: 0-1 (could be anything in the future)
			ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, 0.0, m_TextureResolution - 1.0);
			ImPlot::SetupAxisLimitsConstraints(ImAxis_Y1, 0.0, 1.0);

			// Plot histogram
			if (!m_Histogram.empty())
			{
				ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, .25f);
				ImPlot::PlotShaded("##Histogram", m_XPoints.data(), m_Histogram.data(), m_TextureResolution);
				ImPlot::PopStyleVar();
			}
			

			// Plot control points
			bool isDragging = false;
			bool hasClicked = false;
			int draggedId = -1;

			for (int id = 0; id < m_ControlPoints.size(); ++id)
			{
				isDragging |= ImPlot::DragPoint(id, &m_ControlPoints[id].x, &m_ControlPoints[id].y,
					ImVec4(0, 0.9f, 0, 1), 4, ImPlotDragToolFlags_Delayed, nullptr, nullptr, nullptr);

				if (isDragging && draggedId == -1)
				{
					// This point is being dragged and we will work with it below
					draggedId = id;
					TfUtils::CheckDragBounds(draggedId, m_ControlPoints, m_TextureResolution);
				}
			}

			// Plot transfer function
			ImPlot::PlotLine("Tf", m_XPoints.data(), m_YPoints.data(), m_TextureResolution);

			// Drag event
			if (isDragging)
			{
				UpdateYAxis(draggedId);
			}

			auto dragDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 0.1f);
			hasClicked = ImPlot::IsPlotHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left)
				&& dragDelta.x == 0.0 && dragDelta.y == 0.0;

			// Click event
			if (hasClicked)
			{
				ImPlotPoint mousePos = ImPlot::GetPlotMousePos();

				std::string str = "Clicked in plot:" + std::to_string(static_cast<int>(mousePos.x)) + ", " + std::to_string(mousePos.y);
				LOG_TRACE(str.c_str());

				int index = AddControlPoint(mousePos.x, mousePos.y);
			}
			
			ImGui::InputText("File name", m_NameBuffer, IM_ARRAYSIZE(m_NameBuffer));

			if (ImGui::Button("Save opacity TF preset"))
			{
				std::string n = m_NameBuffer;
				if (n.size() == 0)
				{
					n = "opacityTF";
				}
				std::string str = (FileSystem::GetDefaultPath() / "assets" / n).string();
				Save(str);
			}

			ImPlot::EndPlot();
		}
	}

	void OpacityTF::UpdateTexture()
	{
		if (m_ShouldUpdate)
		{
			assert(p_Texture != nullptr && "Texture is not initialized");
			p_Texture->UpdateTexture(base::GraphicsContext::GetQueue(), m_YPoints.data());
			m_ShouldUpdate = false;
		}
	}

	void OpacityTF::ActivateHistogram(const VolumeFile& file)
	{
		// this function will create histogram of data, (divide either by max value or 2^used bits) then multiplied by desired resolution
		m_Histogram.resize(m_TextureResolution, 0.0f);
		const auto& data = file.GetVecReference();
		auto [xSize, ySize, slices] = file.GetSize();
		const size_t size = xSize * ySize * slices;
		float maxVal = 0.0f;
		// If data are not normalized, normalize and convert to texture range, this is pre-computed factor
		const float factor = m_TextureResolution / file.GetMaxNumber();
		
		for (std::uint32_t i = 0; i < size; ++i)
		{
			// glm::vec4().a -> density (rgb are reserved for gradient)
			int value = 0;
			if (file.IsNormalized())
			{
				// Sometimes the data may be capped, divided by value that is less than the max value in the data, this would go out of bounds
				value = std::min(static_cast<int>(data[i].a * m_TextureResolution), m_TextureResolution - 1);
			}
			else
			{
				value = static_cast<int>(data[i].a * factor);
			}
			
			++m_Histogram[value];
		}
		maxVal = std::log10(size);
		for (float& i : m_Histogram)
		{
			if (i != 0.0f)
			{
				i = std::log10(i) / maxVal;
			}
		}
	}

	bool OpacityTF::Save(const std::string& name)
	{
		// Create file with this structure first line is [opacity] second line is number of control points and then x y values
		std::ofstream file(name);
		file << GetType() << "\n";
		file << "resolution\n";
		file << GetTextureResolution() << "\n";
		file << "data range\n";
		file << GetDataRange() << "\n";
		file << "control points number\n";
		file << m_ControlPoints.size() << "\n";
		for (const auto& cp : m_ControlPoints)
		{
			file << cp.x << " " << cp.y << "\n";
		}
		file.close();
		return true;
	}

	void OpacityTF::Load(const std::string& name)
	{
		// TODO create T TryParse<T> func
		std::ifstream file(name);
		std::string line;
		std::getline(file, line);
		std::vector<glm::dvec2> cps{};

		if (line != GetType())
		{
			LOG_ERROR("Invalid transform function format");
			return;
		}

		// Resolution
		std::getline(file, line);
		if (line != "resolution")
		{
			LOG_ERROR("Wrong format");
			return;
		}

		int resolution = 0;
		try
		{
			std::getline(file, line);
			resolution = std::stoi(line);
		}
		catch (const std::exception& e)
		{
			LOG_ERROR("Invalid resolution");
			return;
		}

		// Data range
		std::getline(file, line);
		if (line != "data range")
		{
			LOG_ERROR("Wrong format");
			return;
		}

		int dataRange = 0;
		{
			try
			{
				std::getline(file, line);
				dataRange = std::stoi(line);
			}
			catch (const std::exception& e)
			{
				LOG_ERROR("Invalid data range");
				return;
			}
		}

		// CPS
		std::getline(file, line);
		if (line != "control points number")
		{
			LOG_ERROR("Wrong format");
			return;
		}

		int controlPoints = 0;
		{
			try
			{
				std::getline(file, line);
				controlPoints = std::stoi(line);
			}
			catch (const std::exception& e)
			{
				LOG_ERROR("Invalid data range");
				return;
			}
		}

		for (int i = 0; i < controlPoints; ++i)
		{
			if (!std::getline(file, line))
			{
				LOG_ERROR("Unexpected end of file, TF was not loaded");
				return;
			}

			std::stringstream linestream(line);
			double x, y;
			linestream >> x >> y;
			cps.emplace_back(x, y);
		}
		file.close();

		// Update the state
		m_ControlPoints = std::move(cps);
		ResolveResolution(resolution);
		// Allocate or destroy mem. if needed
		m_XPoints.resize(m_TextureResolution, 0.0f);
		m_YPoints.resize(m_TextureResolution, 0.0f);
		m_DataRange = dataRange;
		
		// Re-calculate the values betweeb CPs
		for (int i = 0; i < m_ControlPoints.size(); ++i)
		{
			UpdateYAxis(i);
		}

		m_ShouldUpdate = true;
		LOG_INFO("Opacity TF loaded");
	}

	void OpacityTF::CalibrateOnMask(std::shared_ptr<const VolumeFile> mask, std::shared_ptr<const VolumeFile> file, std::array<int, 4> activeContours)
	{
		// Checking
		if (file == nullptr || mask == nullptr)
		{
			LOG_ERROR("TF Calibration: NULLPTR, TF won't be calibrated");
			return;
		}

		auto [x, y, z] = mask->GetSize();
		auto [xx, yy, zz] = file->GetSize();

		// Matching size check
		if (x != xx || y != yy || z != zz)
		{
			LOG_ERROR("TF calibration, mask and data sizes do not match, TF won't be calibrated");
			return;
		}

		size_t size = x * y * z;

		// Empty file check
		if (size == 0)
		{
			LOG_ERROR("TF calibration: empty file, TF won't be calibrated!");
			return;
		}


		size_t maxValue = file->GetMaxNumber();

		const auto& maskData = mask->GetVecReference();
		const auto& fileData = file->GetVecReference();

		assert(maskData.size() == fileData.size() && "Underlying data are not the same size");

		// Max value check
		if (maxValue == 0)
		{
			LOG_WARN("Max value is zero, looking for max value.");
			maxValue = file->GetMaxNumber(fileData, 3); // alpha --> rgb may contain precomputed gradient
			if (maxValue == 0)
			{
				LOG_ERROR("After check: max value is 0, TF won't be calibrated.");
				return;
			}
		}

		std::vector<int> cIndices{};
		for (int i = 0; i < activeContours.size(); ++i)
		{
			if (activeContours[i] == 1)
			{
				cIndices.push_back(i);
			}
		}

		// Check whether user flagged at least one contour
		if (cIndices.size() == 0)
		{
			LOG_ERROR("No contour has been selected for calibration, TF won't be calibrated!"); \
			return;
		}

		// calculate histogram inside the contour
		std::vector<int> bin(maxValue, 0);

		for (size_t i = 0; i < size; ++i)
		{
			for (auto contourIndex : cIndices)
			{
				if (maskData[i][contourIndex] != 0)
				{
					// Active contour
					int value = fileData[i].a;
					++bin[value];
				}
			}
		}
	}

	void OpacityTF::UpdateYAxis(int cpId)
	{
		assert(cpId >= 0 && cpId < m_ControlPoints.size() && "Control point index is out of bounds");
		// Takes x and y coordinate of two control points and execute linear interpolation between them, then copies to resulting array
		auto updateIntervalValues = [&](double cx1, double cx2, float cy1, float cy2)
			{
				int x0 = static_cast<int>(cx1);
				int x1 = static_cast<int>(cx2);

				std::vector<float> result = LinearInterpolation::Generate<float, int>(x0, x1, cy1, cy2, 1);
				assert(result.size() - 1 == std::abs(x1 - x0) && "Size of generated vector does not match");

				for (size_t i = 0; i <= std::abs(x1 - x0); ++i)
				{
					m_YPoints[i + x0] = result[i];
				}
			};

		// Update control interval between control point below and current
		if (cpId - 1 >= 0)
		{
			auto predecessorIndex = static_cast<size_t>(m_ControlPoints[cpId - 1].x);
			updateIntervalValues(m_ControlPoints[cpId - 1].x, m_ControlPoints[cpId].x,
				m_YPoints[predecessorIndex], static_cast<float>(m_ControlPoints[cpId].y));
		}

		// Update control interval between current control point and control point above
		if (cpId + 1 < m_ControlPoints.size())
		{
			auto successorIndex = static_cast<size_t>(m_ControlPoints[cpId + 1].x);
			updateIntervalValues(m_ControlPoints[cpId].x, m_ControlPoints[cpId + 1].x,
				static_cast<float>(m_ControlPoints[cpId].y), m_YPoints[successorIndex]);
		}
		m_ShouldUpdate = true;
	}

	std::string OpacityTF::GetType() const
	{
		return "opacity";
	}
}