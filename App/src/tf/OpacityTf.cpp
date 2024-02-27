#include "OpacityTf.h"
#include "TfUtils.h"
#include "LinearInterpolation.h"
#include "Base/Base.h"
#include "Base/GraphicsContext.h"
#include "implot/implot.h"
#include "implot_internal.h"

#include <cassert>

namespace med
{
	OpacityTF::OpacityTF(int maxDataValue) : m_DataDepth(maxDataValue)
	{
		m_XPoints.resize(m_DataDepth, 0.0f);
		m_YPoints.resize(m_DataDepth, 0.0f);

		m_ControlPoints.emplace_back(0.0, 0.0);
		m_ControlPoints.emplace_back(m_DataDepth - 1.0, 1.0);

		std::vector<float> result = LinearInterpolation::Generate<float>(0, m_DataDepth - 1, 0.0f, 1.0f, 1);
		assert(result.size() == m_DataDepth && "Size of generated TF does not match");

		// Fill for plotting
		for (size_t i = 0; i < m_DataDepth; ++i)
		{
			m_XPoints[i] = i;
			m_YPoints[i] = result[i];
		}

		p_Texture = Texture::CreateFromData(base::GraphicsContext::GetDevice(), base::GraphicsContext::GetQueue(), m_YPoints.data(), WGPUTextureDimension_1D, {m_DataDepth, 1, 1},
			WGPUTextureFormat_R32Float, WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst, sizeof(float), "Opacity TF");
	}

	std::shared_ptr<Texture> OpacityTF::GetTexture() const
	{
		assert(p_Texture != nullptr && "Texture is not initialized");
		return p_Texture;
	}

    // Maybe in the future refactor: HandleClick, HandleDrag
	bool OpacityTF::Render()
	{
        if (ImPlot::BeginPlot("##optfplot"))
        {
            // This sets up axes 1
            ImPlot::SetupAxes("Voxel value", "Alpha");

            // Setup limits, X: 0-4095 (data resolution -- bits per pixel), Y: 0-1 (could be anything in the future)
            ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, 0.0, m_DataDepth - 1.0);
            ImPlot::SetupAxisLimitsConstraints(ImAxis_Y1, 0.0, 1.0);

            // Plot histogram
            if (!m_Histogram.empty())
            {
                ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, .25f);
                ImPlot::PlotShaded("##Histogram", m_XPoints.data(), m_Histogram.data(), m_DataDepth);
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
                    TfUtils::CheckDragBounds(draggedId, m_ControlPoints, m_DataDepth);
                }
            }

            // Plot transfer function
            ImPlot::PlotLine("Tf", m_XPoints.data(), m_YPoints.data(), m_DataDepth);

            // Drag event
            if (isDragging)
            {
                UpdateYPoints(draggedId);
                m_ShouldUpdate = true;
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

                int index = TfUtils::AddControlPoint(mousePos.x, mousePos.y, m_ControlPoints);
                if (index != -1)
                {
					UpdateYPoints(index);
					m_ShouldUpdate = true;
				}
            }

            ImPlot::EndPlot();
        }

		return false;
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
        m_Histogram.resize(m_DataDepth, 0.0f);
        const auto& data = file.GetVecReference();
        auto [xSize, ySize, slices] = file.GetSize();
        const size_t size = xSize * ySize * slices;
        float maxVal = 0.0f;
        for (std::uint32_t i = 0; i < size; ++i)
        {
            // glm::vec4().a -> density (rgb are reserved for gradient)
            auto value = static_cast<int>(data[i].a);
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

    void OpacityTF::UpdateYPoints(int controlPointIndex)
    {
        assert(controlPointIndex >= 0 && controlPointIndex < m_ControlPoints.size() && "Control point index is out of bounds");
        // Takes x and y coordinate of two control points and execute linear interpolation between them, then copies to resulting array
        auto updateIntervalValues = [&](double cx1, double cx2, float cy1, float cy2)
            {
                int x0 = static_cast<int>(cx1);
                int x1 = static_cast<int>(cx2);

                std::vector<float> result = LinearInterpolation::Generate<float, int>(x0, x1, cy1, cy2, 1);
                assert(result.size() - 1 == std::abs(x1 - x0) && "Size of generated vector does not match");

                for (size_t i = 0; i < std::abs(x1 - x0); ++i)
                {
                    m_YPoints[i + x0] = result[i];
                }
            };

        // Update control interval between control point below and current
        if (controlPointIndex - 1 >= 0)
        {
            auto predecessorIndex = static_cast<size_t>(m_ControlPoints[controlPointIndex - 1].x);
            updateIntervalValues(m_ControlPoints[controlPointIndex - 1].x, m_ControlPoints[controlPointIndex].x,
                m_YPoints[predecessorIndex], static_cast<float>(m_ControlPoints[controlPointIndex].y));
        }

        // Update control interval between current control point and control point above
        if (controlPointIndex + 1 < m_ControlPoints.size())
        {
            auto successorIndex = static_cast<size_t>(m_ControlPoints[controlPointIndex + 1].x);
            updateIntervalValues(m_ControlPoints[controlPointIndex].x, m_ControlPoints[controlPointIndex + 1].x,
                static_cast<float>(m_ControlPoints[controlPointIndex].y), m_YPoints[successorIndex]);
        }
    }
}