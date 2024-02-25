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
    std::vector<glm::dvec2> m_ColorMapCPs{}; // 1D would have been enough, but 2D is more flexible for future
};

} // med
