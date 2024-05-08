#pragma once
#include "MiniApp.h"
#include "../../renderer/Texture.h"
#include "../../renderer/BindGroup.h"
#include "../../renderer/PipelineBuilder.h"
#include "../../renderer/RenderPipeline.h"
#include "../../tf/ColorTf.h"
#include "../../tf/OpacityTf.h"
#include "../../renderer/Light.h"

namespace med
{
	class BasicVolLightApp : public MiniApp
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
		BindGroup m_BGroup;
		std::unique_ptr<OpacityTF> p_OpacityTf = nullptr;
		std::unique_ptr<ColorTF> p_ColorTf = nullptr;
		std::shared_ptr<UniformBuffer> p_ULight = nullptr;


		Light m_Light1
		{
			.Position = {0.0, 5.0f, 0.0f, 1.0f},
			.Ambient = glm::vec4{0.1f},
			.Diffuse = glm::vec4{1.0f}
		};

	};
}