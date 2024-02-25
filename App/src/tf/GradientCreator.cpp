//
// Created by krist on 13. 2. 2024.
//

#include "GradientCreator.h"

#include <array>

#include "Base/Base.h"

#include "implot.h"
#include "LinearInterpolation.h"

#include <implot_internal.h>

namespace med
{
    void GradientCreator::Init()
    {
        glm::vec3 color1 = glm::vec3(0.0, 0.0, 0.0);
        glm::vec3 color2 = glm::vec3(1.0, 1.0, 1.0);

        m_Colors = LinearInterpolation::Generate<glm::vec3, int>(0, 4095, color1, color2, 1);
        m_ColorMapCPs.emplace_back(color1.r, color1.g, color1.b, 0.0);
        m_ColorMapCPs.emplace_back(color2.r, color2.g, color2.b, 4095);
    }

    void GradientCreator::Render()
    {
        if (ImPlot::BeginPlot("##gradient", ImVec2(-1, -1)))
        {
            ImPlot::SetupAxes(nullptr, nullptr, 0, ImPlotAxisFlags_NoDecorations);
            ImPlot::SetupAxisLimitsConstraints(ImAxis_Y1, 0.0, 1.0);
            ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, 0.0, 4095.0);
            // Out plot is from 0 to 4095 on x axis and 0 to 1 on y axis
            ImVec2 rmin = ImPlot::PlotToPixels(ImPlotPoint(0.0, 0.0f));
            ImVec2 rmax = ImPlot::PlotToPixels(ImPlotPoint(1.0f, 1.0f));
            
            float xx = 1.0f;

            ImPlot::PushPlotClipRect();
            for (const auto& rect : m_Colors)
            {
				ImPlot::GetPlotDrawList()->AddRectFilled(ImPlot::PlotToPixels(ImPlotPoint(xx - 1.0f, 0.0f)), ImPlot::PlotToPixels(ImPlotPoint(xx, 1.0f)),
                    IM_COL32(255 * rect.r, 255 * rect.g, 255 * rect.b, 255));
                xx += 1.0f;
                // ImPlot::GetPlotDrawList()->AddRectFilled(rmin, rmax, ImGui::ColorConvertFloat4ToU32(ImVec4(rect.x, rect.y, rect.z, 1.0f));
			}
            
            int clickedId = -1;
            bool hasClicked = false;
            // We don't care about y coordinate, so we can use the same variable
            double HEIGHT = 0.5;
            for (int id = 0; id < m_ColorMapCPs.size(); ++id)
            {
                ImPlot::DragPoint(id, &m_ColorMapCPs[id].a, &HEIGHT, ImVec4(255, 255, 0, 255), 4.0f, ImPlotDragToolFlags_Delayed);
                HEIGHT = 0.5;
            }

            ImPlot::PopPlotClipRect();

            ImPlot::EndPlot();
        }
    
    }

} // med
