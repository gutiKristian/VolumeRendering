#pragma once
#include <cstdint>
#include <imgui/imgui.h>
#include "implot.h"
#include "implot_internal.h"

#include "Base/Timestep.h"
#include "../../renderer/PipelineBuilder.h"
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
			m_StepsCount = max;
		}

		float GetStepSize() const { return m_StepSize; }
		int GetStepsCount() const { return m_StepsCount; }
	protected:
		float m_StepSize = 0.0f;
		int m_StepsCount = 0;
	};
}