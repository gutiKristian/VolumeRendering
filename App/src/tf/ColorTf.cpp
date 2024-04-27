#include "ColorTf.h"
#include "LinearInterpolation.h"
#include "implot.h"
#include "imgui.h"
#include "Base/Base.h"
#include "Base/GraphicsContext.h"
#include "TfUtils.h"
#include "../file/FileSystem.h"
#include <glm/gtc/type_ptr.hpp>

#include <cassert>
#include <fstream>
#include <sstream>

namespace med
{

	ColorTF::ColorTF(int desiredTfResolution)
	{
		m_TextureResolution = desiredTfResolution;
		size_t maxTex1DSize = base::GraphicsContext::GetLimits().maxTextureDimension1D;
		if (m_TextureResolution > maxTex1DSize)
		{
			LOG_WARN("Max value is greater than maximum texture 1D size, clamping to maximum size");
			m_TextureResolution = maxTex1DSize;
		}

		auto color1 = glm::vec4(0.0, 0.0, 0.0, 1.0);
		auto color2 = glm::vec4(1.0, 1.0, 1.0, 1.0);

		m_Colors = LinearInterpolation::Generate<glm::vec4, int>(0, m_TextureResolution - 1, color1, color2, 1);

		m_ControlCol.push_back(color1);
		m_ControlCol.push_back(color2);

		m_ControlPos.emplace_back(0.0, 0.5);
		m_ControlPos.emplace_back(m_TextureResolution - 1.0, 0.5);

		p_Texture = Texture::CreateFromData(base::GraphicsContext::GetDevice(), base::GraphicsContext::GetQueue(), m_Colors.data(), WGPUTextureDimension_1D, {m_TextureResolution, 1, 1},
						WGPUTextureFormat_RGBA32Float, WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst, sizeof(glm::vec4), "Color TF");
	}
		
	void ColorTF::Render()
	{
		assert(m_ControlCol.size() == m_ControlPos.size() && "Number of control points colors don't match with number of positions");
		auto cpSize = m_ControlCol.size();

		if (ImPlot::BeginPlot("##gradient", ImVec2(-1, 150), ImPlotFlags_NoMenus | ImPlotFlags_NoBoxSelect | ImPlotFlags_NoFrame))
		{
			ImPlot::SetupAxes(nullptr, nullptr, 0, ImPlotAxisFlags_NoDecorations);
			ImPlot::SetupAxisLimitsConstraints(ImAxis_Y1, 0.0, 1.0);
			ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, 0.0, m_TextureResolution - 1);

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
					if (id == 0 || id == (cpSize - 1))
					{
						// We don't want to drag the first and last control points
						isDragging = false;
						m_ControlPos[0].x = 0.0;
						m_ControlPos[cpSize - 1].x = m_TextureResolution - 1.0;
					}
					else
					{
						// This point is being dragged and we will work with it below
						draggedId = id;
						TfUtils::CheckDragBounds(draggedId, m_ControlPos, m_TextureResolution);
					}
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
				int index = TfUtils::AddControlPoint(x, 0.5, m_ControlPos);
				if (index != -1)
				{
					m_ControlCol.emplace(m_ControlCol.begin() + index, m_Colors[static_cast<int>(x)]);
					UpdateColors(index);
				}
				LOG_INFO("Added colormap cp");
			}

			if (ImGui::Button("Save Color TF preset"))
			{
				std::string s = (FileSystem::GetDefaultPath() / "assets" / "colormap1").string();
				Save(s);
			}

			ImPlot::EndPlot();
		}
	}

	void ColorTF::UpdateTexture()
	{
		if (m_ShouldUpdate)
		{
			assert(p_Texture != nullptr && "Texture is not initialized");
			p_Texture->UpdateTexture(base::GraphicsContext::GetQueue(), m_Colors.data());
			m_ShouldUpdate = false;
		}
	}

	void ColorTF::Save(const std::string& name)
	{
		assert(m_ControlCol.size() == m_ControlPos.size() && "Number of control points colors don't match with number of positions");
		// Create file with this structure first line is [opacity] second line is number of control points and then x y values
		std::ofstream file(name);
		file << "[color]\n";
		file << m_ControlPos.size() << "\n";
		for (int i = 0; i < m_ControlCol.size(); ++i)
		{
			file << m_ControlPos[i].x << " " << m_ControlPos[i].y << m_ControlCol[i].r << " " << m_ControlCol[i].g << " " << m_ControlCol[i].b << " " << m_ControlCol[i].a << "\n";
		}
		file.close();
	}

	void ColorTF::Load(const std::string& name)
	{
		std::ifstream file(name);
		std::string line;
		std::vector<glm::dvec2> cpPos{};
		std::vector<glm::vec4> cpCol{};

		std::getline(file, line);
		
		if (line != "[color]")
		{
			LOG_ERROR("Invalid file format");
			return;
		}

		std::getline(file, line);
		int controlPoints = 0;
		try
		{
			controlPoints = std::stoi(line);
		}
		catch (const std::exception&)
		{
			LOG_ERROR("Invalid file format");
			return;
		}

		for (int i = 0; i < controlPoints; ++i)
		{
			if (!std::getline(file, line))
			{
				LOG_ERROR("Unexpected end of file, TF was not loaded");
				return;
			}

			std::istringstream linestream(line);
			double x, y;
			glm::vec4 color;

			linestream >> x >> y; 
			linestream >> color.r >> color.g >> color.b >> color.a;
			cpPos.emplace_back(x, y);
			cpCol.push_back(color);
		}
		file.close();
		
		m_ControlCol = cpCol;
		m_ControlPos = cpPos;

		// Jumping by 2 to avoid recalculation of the same interval
		for (int i = 1; i < controlPoints; i+=2)
		{
			UpdateColors(i);
		}

		m_ShouldUpdate = true;
		LOG_INFO("Color TF loaded");
	}

	void ColorTF::UpdateColors(int cpId)
	{
		assert(cpId >= 0 && cpId < m_ControlCol.size() && "Control point index is out of bounds");

		// Takes x and y coordinate of two control points and execute linear interpolation between them, then copies to resulting array
		auto updateIntervalValues = [&](double cx1, double cx2, glm::vec4 cy1, glm::vec4 cy2)
			{
				int x0 = static_cast<int>(cx1);
				int x1 = static_cast<int>(cx2);
				
				std::vector<glm::vec4> result = LinearInterpolation::Generate<glm::vec4, int>(x0, x1, cy1, cy2, 1);
				assert(result.size() - 1 == std::abs(x1 - x0) && "Size of generated vector does not match");

				for (size_t i = 0; i <= std::abs(x1 - x0); ++i)
				{
					m_Colors[i + x0] = result[i];
				}
			};

		// Update control interval between control point below and current
		if (cpId - 1 >= 0)
		{
			auto predecessorIndex = static_cast<int>(m_ControlPos[cpId - 1].x);
			updateIntervalValues(m_ControlPos[cpId - 1].x, m_ControlPos[cpId].x, m_Colors[predecessorIndex], m_ControlCol[cpId]);
		}

		// Update control interval between current control point and control point above
		if (cpId + 1 < m_ControlCol.size())
		{
			auto successorIndex = static_cast<int>(m_ControlPos[cpId + 1].x);
			updateIntervalValues(m_ControlPos[cpId].x, m_ControlPos[cpId + 1].x, m_ControlCol[cpId], m_Colors[successorIndex]);
		}
		m_ShouldUpdate = true;
	}

	std::string ColorTF::GetType() const
	{
		return "color";
	}
} // namespace med