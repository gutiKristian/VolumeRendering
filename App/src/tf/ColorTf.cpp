#include "ColorTf.h"
#include "LinearInterpolation.h"
#include "implot.h"
#include "imgui.h"
#include "Base/Base.h"
#include "Base/GraphicsContext.h"
#include "TfUtils.h"

#include <cassert>
#include <glm/gtc/type_ptr.hpp>

namespace med
{
	ColorTF::ColorTF(int maxDataValue) : m_DataDepth(maxDataValue)
	{
		glm::vec3 color1 = glm::vec3(0.0, 0.0, 0.0);
		glm::vec3 color2 = glm::vec3(1.0, 1.0, 1.0);

		m_Colors = LinearInterpolation::Generate<glm::vec3, int>(0, 4095, color1, color2, 1);

		m_ControlCol.emplace_back(color1.r, color1.g, color1.b, 1.0);
		m_ControlCol.emplace_back(color2.r, color2.g, color2.b, 1.0);

		m_ControlPos.emplace_back(0.0, 0.5);
		m_ControlPos.emplace_back(m_DataDepth - 1.0, 0.5);
	}
	
	std::shared_ptr<Texture> ColorTF::GetTexture() const
	{
		assert(p_Texture != nullptr && "Texture is not initialized");
		return p_Texture;
	}
	
	bool ColorTF::Render()
	{
		assert(m_ControlCol.size() == m_ControlPos.size() && "Number of control points colors don't match with number of positions");
		auto cpSize = m_ControlCol.size();

		if (ImPlot::BeginPlot("##gradient", ImVec2(-1, -1), ImPlotFlags_NoMenus | ImPlotFlags_NoBoxSelect | ImPlotFlags_NoFrame))
		{
			ImPlot::SetupAxes(nullptr, nullptr, 0, ImPlotAxisFlags_NoDecorations);
			ImPlot::SetupAxisLimitsConstraints(ImAxis_Y1, 0.0, 1.0);
			ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, 0.0, 4095.0);

			// Plot is from 0 to 4095 on x axis and 0 to 1 on y axis
			float xx = 1.0f;
			ImPlot::PushPlotClipRect();
			for (const auto& rect : m_Colors)
			{
				ImPlot::GetPlotDrawList()->AddRectFilled(ImPlot::PlotToPixels(ImPlotPoint(xx - 1.0f, 0.0f)), ImPlot::PlotToPixels(ImPlotPoint(xx, 1.0f)),
					IM_COL32(255 * rect.r, 255 * rect.g, 255 * rect.b, 255));
				xx += 1.0f;
				// ImPlot::GetPlotDrawList()->AddRectFilled(ImPlot::PlotToPixels(ImPlotPoint(xx - 1.0f, 0.0f)), ImPlot::PlotToPixels(ImPlotPoint(xx, 1.0f)), ImGui::ColorConvertFloat4ToU32(ImVec4(rect.x, rect.y, rect.z, 1.0f));
			}

			bool isDragging = false;
			int draggedId = -1;
			// We don't care about y coordinate, so we can use the same variable
			double HEIGHT = 0.5;
			for (int id = 0; id < cpSize; ++id)
			{
				bool clickEvent = false;
				bool held = false;
				isDragging |= ImPlot::DragPoint(id, &m_ControlPos[id].x, &HEIGHT, ImVec4(0, 0, 255, 255), 4.0f, ImPlotDragToolFlags_Delayed, &clickEvent);
				m_HasClicked |= clickEvent;

				if (isDragging && draggedId == -1)
				{
					// This point is being dragged and we will work with it below
					draggedId = id;
					TfUtils::CheckDragBounds(draggedId, m_ControlPos, m_DataDepth);
				}

				if (clickEvent)
				{
					m_ClickedCpId = id;
				}

				HEIGHT = 0.5;
			}

			if (isDragging)
			{
				assert(draggedId != -1 && "Dragging event was unexpectedly fired and dragged id is invalid");
				// Recalculate colors
				UpdateColors(draggedId);
			}

			// Do not open pop up if we are dragging or dragging has been done
			if (m_HasClicked && !isDragging)
			{
				ImGui::OpenPopup("##colorpick");
				m_HasClicked = false;
			}
			if (ImGui::BeginPopup("##colorpick"))
			{
				assert(m_ClickedCpId != -1 && "Clicked control point id is invalid");
				if (ImGui::ColorPicker4("Color", glm::value_ptr(m_ControlCol[m_ClickedCpId])))
				{
					// Recalculate colors
					UpdateColors(m_ClickedCpId);
				}
				ImGui::EndPopup();
			}
			else
			{
				m_ClickedCpId = -1;
			}

			ImPlot::PopPlotClipRect();

			// Add new control point, if right mouse button is clicked, on purpose at the end
			// of the function, so it's drawn next frame
			if (ImPlot::IsPlotHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			{
				auto x = std::round(ImPlot::GetPlotMousePos().x);
				glm::vec3 color = m_Colors[static_cast<int>(x)];
				int index = TfUtils::AddControlPoint(x, 0.5, m_ControlPos);
				if (index != -1)
				{
					m_ControlCol.emplace(m_ControlCol.begin() + index, color.r, color.g, color.b, 1.0);
				}
				LOG_INFO("Added colormap cp");
			}

			ImPlot::EndPlot();
		}

		return true;
	}

	void ColorTF::UpdateTexture()
	{
	}

	void ColorTF::UpdateColors(int cpId)
	{
	}
} // namespace med