#pragma once
#include <cstdint>
#include <imgui/imgui.h>
#include "implot.h"
#include "implot_internal.h"

#include "Base/Timestep.h"
#include "../../renderer/Texture.h"
#include "../../renderer/BindGroup.h"
#include "../../renderer/PipelineBuilder.h"
#include "../../renderer/RenderPipeline.h"
#include "../../tf/ColorTf.h"
#include "../../tf/OpacityTf.h"
#include "../../renderer/Light.h"
#include "../../file/VolumeFile.h"

#define MED_BEGIN_TAB_BAR(name) \
	if (ImGui::BeginTabBar(name)) \
	{

#define MED_END_TAB_BAR \
		ImGui::EndTabBar(); \
	}

#define MED_BEGIN_TAB_ITEM(name) \
	if (ImGui::BeginTabItem(name)) \
	{

#define MED_END_TAB_ITEM \
		ImGui::EndTabItem(); \
	}

namespace med
{
	class MiniApp
	{
	public:
		virtual void OnStart(PipelineBuilder& pipeline) = 0;
		virtual void OnUpdate(base::Timestep ts) = 0;
		virtual void OnRender(WGPURenderPassEncoder pass) = 0;
		virtual void OnEnd() = 0;
		virtual void OnImGuiRender() const = 0;
		virtual void OnResize(uint32_t width, uint32_t height, PipelineBuilder& pipeline) { IntializePipeline(pipeline); }
		virtual void IntializePipeline(PipelineBuilder& pipeline) = 0;
		void ComputeRecommendedSteppingParams(const VolumeFile& file)
		{
			auto [x, y, z] = file.GetSize();
			int max = std::max(x, std::max(y, z));
			m_StepSize = 1.0f / static_cast<float>(max);
			// If we would have used unit cube, the length of the longest possible ray is sqrt(3),
			// therefore we wouldn't be able to sample whole volume with stepsize 1 / max
			m_StepsCount = std::sqrt(3) * max;
		}

		float GetStepSize() const { return m_StepSize; }
		int GetStepsCount() const { return m_StepsCount; }
	protected:
		float m_StepSize = 0.0f;
		int m_StepsCount = 0;
	};
}