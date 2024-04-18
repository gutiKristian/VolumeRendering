#pragma once
#include "MiniApp.h"

namespace med
{
	class DicomInfoApp : public MiniApp
	{
	public:
		void OnStart(PipelineBuilder& pipeline) override;
		void OnUpdate(base::Timestep ts) override;
		void OnRender(WGPURenderPassEncoder pass) override;
		void OnEnd() override;
		void OnImGuiRender() const override;
		void IntializePipeline(PipelineBuilder& pipeline) override;
	};
}