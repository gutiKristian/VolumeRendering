#pragma once
#include "MiniApp.h"

namespace med
{
	class MultiCTRTApp : public MiniApp
	{
		void OnStart(PipelineBuilder& pipeline) override;
		void OnUpdate(base::Timestep ts) override;
		void OnRender(WGPURenderPassEncoder pass) override;
		void OnEnd() override;
		void OnImGuiRender() const override;
		void IntializePipeline(PipelineBuilder& pipeline) override;

	private:
		std::shared_ptr<Texture> p_TexCTData = nullptr;
		std::shared_ptr<Texture> p_TexRTData = nullptr;
		std::shared_ptr<Texture> p_TexMaskData = nullptr;
		BindGroup m_BGroup;
		std::unique_ptr<OpacityTF> p_OpacityTfCT = nullptr;
		std::unique_ptr<OpacityTF> p_OpacityTfRT = nullptr;
		std::unique_ptr<ColorTF> p_ColorTfCT = nullptr;
		std::unique_ptr<ColorTF> p_ColorTfRT = nullptr;
		std::shared_ptr<UniformBuffer> p_ULight = nullptr;

		Light m_Light1
		{
			.Position = {5.0f, 5.0f, -5.0f, 1.0f},
			.Ambient = glm::vec4{0.1f},
			.Diffuse = glm::vec4(1.0f)
		};
	};
}