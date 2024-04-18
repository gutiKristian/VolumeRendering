#include "MiniApp.h"
#include "../../renderer/Texture.h"
#include "../../renderer/BindGroup.h"
#include "../../renderer/PipelineBuilder.h"
#include "../../renderer/RenderPipeline.h"
#include "../../tf/ColorTf.h"
#include "../../tf/OpacityTf.h"

namespace med
{
	class ThreeFilesApp : public MiniApp
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
		std::shared_ptr<Texture> p_TexCTData = nullptr;
		std::shared_ptr<Texture> p_TexRTData = nullptr;
		std::shared_ptr<Texture> p_TexMaskData = nullptr;
		BindGroup m_BGroup;
		std::unique_ptr<OpacityTF> p_OpacityTfCT = nullptr;
		std::unique_ptr<OpacityTF> p_OpacityTfRT = nullptr;
		std::unique_ptr<ColorTF> p_ColorTfCT = nullptr;
		std::unique_ptr<ColorTF> p_ColorTfRT = nullptr;
		std::shared_ptr<UniformBuffer> p_ULight = nullptr;
		

		// Directional, although we need only rgb, padding is added for alignment -> wgpu aligns to 16 bytes
		struct Light
		{
			glm::vec4 Position{ 0.0f };
			glm::vec4 Ambient{ 0.0f };
			glm::vec4 Diffuse{ 0.0f };
			glm::vec4 Specular{ 0.0f };
		};

		Light m_Light1
		{
			.Position = {5.0f, 5.0f, -5.0f, 1.0f},
			.Ambient = glm::vec4{0.1f},
			.Diffuse = glm::vec4(1.0f),
			.Specular = glm::vec4(1.0f)
		};

	};
}