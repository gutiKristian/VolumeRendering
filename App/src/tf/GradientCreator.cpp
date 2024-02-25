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
        // Default
        m_ControlPoints.emplace_back(0.0f, 0.0f, 0.0f, 0.0f);
        m_ControlPoints.emplace_back(1.0f, 1.0f, 1.0f, 1.0f);

        m_ColorsP = LinearInterpolation::Generate<glm::vec3, int>(0, 4095, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), 1);

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
            for (const auto& rect : m_ColorsP)
            {
				ImPlot::GetPlotDrawList()->AddRectFilled(ImPlot::PlotToPixels(ImPlotPoint(xx - 1.0f, 0.0f)), ImPlot::PlotToPixels(ImPlotPoint(xx, 1.0f)),
                    IM_COL32(255 * rect.r, 255 * rect.g, 255 * rect.b, 255));
                xx += 1.0f;
                // ImPlot::GetPlotDrawList()->AddRectFilled(rmin, rmax, ImGui::ColorConvertFloat4ToU32(ImVec4(rect.x, rect.y, rect.z, 1.0f));
			}


            ImPlot::PopPlotClipRect();

            ImPlot::EndPlot();
        }
    
        auto *g = ImGui::GetCurrentContext();

        if (g == nullptr || g->CurrentWindow->SkipItems)
            return;

        auto &gp = *ImPlot::GetCurrentContext();
        // Calculates the width and height of colormap that will be rendered
        const ImVec2 pos = ImGui::GetCurrentWindow()->DC.CursorPos;
        const float w = ImGui::CalcItemWidth();
        const float h = ImGui::GetFrameHeight();

        // Calculates how far can we go in x and y (this stretches it)
        const ImVec2 maxExt = ImGui::CalcItemSize(ImVec2(-1, -1), w, h);

        ImRect rect = ImRect(pos.x, pos.y, pos.x + maxExt.x, pos.y + h);

        RenderColorBar(*ImGui::GetWindowDrawList(), rect, false, true);
        
       
        /*ImPlot::ColormapSlider
        ImPlot::DragPoint*/

        RenderHandlers(rect.GetWidth());
    }

    void GradientCreator::RenderColorBar(ImDrawList &drawList, const ImRect &bounds, bool isReversed,
                                         bool isContinuos) const
    {
        auto size = static_cast<int>(m_ControlPoints.size());
        const int n = isContinuos ? size - 1 : size;
        ImU32 col1, col2;

        const float step = bounds.GetWidth() / n;
        ImRect rect(bounds.Min.x, bounds.Min.y, bounds.Min.x + step, bounds.Max.y);

        for (int i = 0; i < n; ++i)
        {
            col1 = IM_COL32(255 * m_ControlPoints[i].x, 255 * m_ControlPoints[i].y, 255 * m_ControlPoints[i].z, 255);
            col2 = isContinuos
                       ? IM_COL32(255 * m_ControlPoints[i+1].x, 255 * m_ControlPoints[i+1].y,
                                  255 * m_ControlPoints[i+1].z, 255)
                       : col1;
            drawList.AddRectFilledMultiColor(rect.Min, rect.Max, col1, col2, col2, col1);
            rect.TranslateX(step);
        }

        // This also moves cursor
        if (ImGui::InvisibleButton("##", ImVec2(bounds.GetWidth(), bounds.GetHeight())))
        {
            LOG_TRACE("Adding gradient handle point");
        }
    }

    void GradientCreator::RenderHandle(const ImVec4 cp, float boxWidth) const
    {
        auto *window = ImGui::GetCurrentWindow();

        ImVec2 position = window->DC.CursorPos;
        position.x += boxWidth * cp.w; // in .w or also as .a the local position is stored

        std::array<ImVec2, 3> tPoints{
            position, ImVec2(position.x - TRIANGLE_WIDTH, position.y + TRIANGLE_HEIGHT),
            ImVec2(position.x + TRIANGLE_WIDTH, position.y + TRIANGLE_HEIGHT)
        };
        // Render the triangle
        window->DrawList->AddConvexPolyFilled(tPoints.data(), tPoints.size(), IM_COL32(255 * cp.x,
                                                  255 * cp.y, 255 * cp.z, 255));

        // Reposition curosor lower, starting y pos of button for this triangle
        window->DC.CursorPos.y = position.y + TRIANGLE_HEIGHT;
        window->DC.CursorPos.x = position.x - TRIANGLE_WIDTH;
        // Render button under
        ImGui::ColorButton("##Display", cp, ImGuiColorEditFlags_NoInputs,
                           ImVec2(TRIANGLE_WIDTH * 2.0f, TRIANGLE_HEIGHT * 2.0f));
        // Restore position
        window->DC.CursorPos.y -= TRIANGLE_HEIGHT;
        window->DC.CursorPos.x += TRIANGLE_WIDTH;
    }

    void GradientCreator::RenderHandlers(float boxWidth) const
    {
        for (const auto &cp: m_ControlPoints)
        {
            ImVec2 pos(0.0f, 0.0f);
            RenderHandle(cp, boxWidth);
        }
    }
} // med
