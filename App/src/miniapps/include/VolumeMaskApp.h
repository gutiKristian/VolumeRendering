#pragma once
#include "MiniApp.h"
#include "../../renderer/Texture.h"
#include "../../renderer/BindGroup.h"
#include "../../renderer/PipelineBuilder.h"
#include "../../renderer/RenderPipeline.h"
#include "../../tf/ColorTf.h"
#include "../../tf/OpacityTf.h"

namespace med
{
	class VolumeMaskApp : public MiniApp
	{
	public:
		void OnStart(PipelineBuilder& pipeline) override;
		void OnUpdate(base::Timestep ts) override;
		void OnRender(const WGPURenderPassEncoder pass) override;
		void OnEnd() override;
		void OnImGuiRender() const override;
		void OnResize(uint32_t width, uint32_t height, PipelineBuilder& pipeline);
		void IntializePipeline(PipelineBuilder& pipeline) override;

	private:
		std::shared_ptr<Texture> p_TexData = nullptr;
		std::shared_ptr<Texture> p_CTTexData = nullptr;
		std::shared_ptr<Texture> p_RTTexData = nullptr;
		std::unique_ptr<OpacityTF> p_OpacityTfCT = nullptr;
		std::unique_ptr<OpacityTF> p_OpacityTfRT = nullptr;
		std::unique_ptr<ColorTF> p_ColorTfCT = nullptr;
		std::unique_ptr<ColorTF> p_ColorTfRT = nullptr;

		BindGroup m_BGroup;
	};
}