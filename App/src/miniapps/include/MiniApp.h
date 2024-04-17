#include <cstdint>
#include "../../renderer/PipelineBuilder.h"

namespace med
{
	class MiniApp
	{
	public:
		virtual void OnStart(PipelineBuilder& pipeline) = 0;
		virtual void OnUpdate() = 0;
		virtual void OnRender(WGPURenderPassEncoder pass) = 0;
		virtual void OnEnd() = 0;
		virtual void OnImGuiRender() const = 0;
		virtual void OnResize(uint32_t width, uint32_t height, PipelineBuilder&) {}
		virtual void IntializePipeline(PipelineBuilder& pipeline) = 0;
	};
}