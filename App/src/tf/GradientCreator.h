#pragma once

#include <vector>

#include <imgui_internal.h>
#include "imgui.h"



namespace med
{

class GradientCreator
{
public:
    void Init();
    void Render();
private:
    void RenderColorBar(ImDrawList& drawList, const ImRect& bounds, bool isReversed, bool isContinuos) const;
    /**
     * \brief Render handle (triangle for better positioning and a button)
     * \param cp control point, xyz has information about color and alpha is position where 0-start and 1-end
     * \param boxWidth width of the colorramp in pixels, serves as offset, cp.a=0.5, cp.a * boxWidth is cp in the middle
     */
    void RenderHandle(const ImVec4 cp, float boxWidth) const;

    void RenderHandlers(float boxWidth) const;
private:
    std::vector<ImU32> m_Colors{};
    // RGB color, Alpha represents value within the color ramp [0,1]
    std::vector<ImVec4> m_ControlPoints{};
private:
    // bit misleading, real width is TRIANGLE_WIDTH * 2, but it is shorter and better when offseting
    const float TRIANGLE_WIDTH = 5.0f;
    const float TRIANGLE_HEIGHT = 5.0f;
};

} // med
