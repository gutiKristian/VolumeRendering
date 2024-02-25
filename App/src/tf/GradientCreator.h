#pragma once

#include <vector>

#include <imgui_internal.h>
#include "imgui.h"
#include "glm/glm.hpp"

namespace med
{

class GradientCreator
{
public:
    void Init();
    void Render();
private:
    std::vector<glm::vec3> m_Colors{};
    // RGB and position (only x is used, y is always 0.5)
    std::vector<glm::dvec4> m_ColorMapCPs{};
};

} // med
