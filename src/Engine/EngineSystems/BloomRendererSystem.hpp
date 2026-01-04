#pragma once

#include "../Core/Scene.hpp"
#include "../EngineComponents/RenderableMesh.hpp"

class BloomRendererSystem : public RendererSystem
{
private:
	std::vector<FrameBufferHandle> m_targets;

	FrameBufferHandle m_mainTarget;

	ShaderHandle m_screenShader;
	ShaderHandle m_thresholdShader;
	ShaderHandle m_downSamplerShader;

	VertexArrayHandle m_screenQuad;
public:
	BloomRendererSystem(size_t levels = 6) 
	{
		m_targets.resize(levels);
	}

	virtual void OnStart(Scene& scene, RenderDevice& renderDevice) override
	{
		/*m_target = scene.CreateFrameBuffer(scene.GraphicsMode().Width, scene.GraphicsMode().Height,
		{
			AttachmentInfo(InternalFormat::RGB8, Format::RGB, MinFilterMode),
		}, 
		false);*/



		uint32_t width  = scene.SelectedGraphicsMode().Width;
		uint32_t height = scene.SelectedGraphicsMode().Height;

		m_mainTarget = scene.CreateFrameBuffer(width, height,
		{
			AttachmentInfo(InternalImageFormat::RGB32F, ImageFormat::RGB, ElementType::Float32, MinFilterMode::Linear, MagFilterMode::Linear, TextureWrappingMode::ClampedToEdge),
		}, {});

		for(size_t i = 0; i < m_targets.size(); i++)
		{
			m_targets[i] = scene.CreateFrameBuffer(width, height,
			{
				AttachmentInfo(InternalImageFormat::RGB32F, ImageFormat::RGB, ElementType::Float32, MinFilterMode::Linear, MagFilterMode::Linear, TextureWrappingMode::ClampedToEdge),
			},  {});

			width  /= 2;
			height /= 2;
		}

		     m_screenShader = scene.LoadShader("screen.glsl", "screen_FS.glsl");
		  m_thresholdShader = scene.LoadShader("threshold_VS.glsl", "threshold_FS.glsl");
		m_downSamplerShader = scene.LoadShader("gaussianBlur_VS.glsl", "gaussianBlur_FS.glsl");

		Array<ScreenVertex> vertices = 
		{
			glm::vec2(-1, -1), 
			glm::vec2( 1, -1),
			glm::vec2(-1,  1),
			glm::vec2( 1,  1),
		};

		std::vector<uint32_t> indices = 
		{ 
			0, 2, 1,
			1, 2, 3
		};

		m_screenQuad = renderDevice.CreateMesh(Model(vertices, indices))->CreateVertexArray(BufferLayout());
	}

	virtual void OnRender(Scene& scene, RenderDevice& renderDevice, RenderContext2D& renderContext2D, FrameBufferHandle target) override
	{
		renderDevice.Disable(RenderFlags::DepthTest);

		m_targets[0]->Bind();
		m_thresholdShader->SetTexture("u_source", target->GetColorAttachment(0));
		m_thresholdShader->Get<float>("u_threshold") = 1.3f;
		m_thresholdShader->Get<int>("u_levels") = int(m_targets.size());
		m_thresholdShader->Use(nullptr);
		m_screenQuad->Draw(nullptr);

		for(size_t i = 1; i < m_targets.size(); i++)
		{
			TextureHandle source = m_targets[i - 1]->GetColorAttachment(0);

			m_targets[i]->Bind();
			m_downSamplerShader->SetTexture("u_source", source);
			m_downSamplerShader->Get<glm::vec2>("u_sourceSize") = glm::vec2(source->Width, source->Height);
			m_downSamplerShader->Use(nullptr);
			m_screenQuad->Draw(nullptr);
		}

		target->Bind();
		
		renderDevice.Enable(RenderFlags::Blending);
		renderDevice.SetBlendFunction(BlendFactor::One, BlendFactor::One);

		for(size_t i = m_targets.size() - 1; i < m_targets.size(); i++)
		{
			m_screenShader->SetTexture("u_source", m_targets[i]->GetColorAttachment(0));
			m_screenShader->Use(nullptr);
			m_screenQuad->Draw(nullptr);
		}

		renderDevice.Disable(RenderFlags::Blending);
		renderDevice.Enable(RenderFlags::DepthTest);
	}
};