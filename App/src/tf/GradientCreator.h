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
    // Control points positions and colors
    std::vector<glm::dvec2> m_CmCpPos{};
    std::vector<glm::vec4> m_CmCpColor{};

    int m_ClickedCpId = -1;
};

} // med
