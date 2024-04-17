#include <cstdint>
#include <imgui/imgui.h>
#include "implot.h"
#include "implot_internal.h"

#include "Base/Timestep.h"
#include "../../renderer/PipelineBuilder.h"

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
		virtual void OnResize(uint32_t width, uint32_t height, PipelineBuilder&) {}
		virtual void IntializePipeline(PipelineBuilder& pipeline) = 0;
	};
}