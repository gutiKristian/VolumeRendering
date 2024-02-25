//
// Created by krist on 13. 2. 2024.
//

#include "GradientCreator.h"

#include <array>

#include "Base/Base.h"

#include "implot.h"
#include "LinearInterpolation.h"
#include <glm/gtc/type_ptr.hpp>
#include <implot_internal.h>


namespace med
{
    void GradientCreator::Init()
    {
        glm::vec3 color1 = glm::vec3(0.0, 0.0, 0.0);
        glm::vec3 color2 = glm::vec3(1.0, 1.0, 1.0);

        m_Colors = LinearInterpolation::Generate<glm::vec3, int>(0, 4095, color1, color2, 1);
        
        m_CmCpColor.emplace_back(color1.r, color1.g, color1.b, 1.0);
        m_CmCpColor.emplace_back(color2.r, color2.g, color2.b, 1.0);

        m_CmCpPos.emplace_back(0.0, 0.5);
        m_CmCpPos.emplace_back(4095.0, 0.5);
    }

    void GradientCreator::Render()
    {
        assert(m_CmCpColor.size() == m_CmCpPos.size() && "Number of control points colors don't match with number of positions");
        auto cpSize = m_CmCpColor.size();

        if (ImPlot::BeginPlot("##gradient", ImVec2(-1, -1)))
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
            static bool hasClicked = false;
            // We don't care about y coordinate, so we can use the same variable
            double HEIGHT = 0.5;
            for (int id = 0; id < cpSize; ++id)
            {
                bool clickEvent = false;
                isDragging |= ImPlot::DragPoint(id, &m_CmCpPos[id].x, &HEIGHT, ImVec4(255, 255, 0, 255), 4.0f, ImPlotDragToolFlags_Delayed, &clickEvent);
                hasClicked |= clickEvent;
                
                if (clickEvent)
                {
                    m_ClickedCpId = id;
                }

                HEIGHT = 0.5;
            }

            if (isDragging)
            {
                // Recalculate colors

                isDragging = false;
            }

            if (hasClicked)
            {
               ImGui::OpenPopup("##colorpick");
               hasClicked = false;
            }
            if (ImGui::BeginPopup("##colorpick"))
            {
                assert(m_ClickedCpId != -1 && "Clicked control point id is invalid");
                if (ImGui::ColorPicker4("Color", glm::value_ptr(m_CmCpColor[m_ClickedCpId])))
                {
                    // Recalculate colors
                }
                ImGui::EndPopup();
            }
            else
            {
                m_ClickedCpId = -1;
            }


            ImPlot::PopPlotClipRect();
            ImPlot::EndPlot();
        }
    
    }

} // med
