#pragma once

#include "../Rendering/RenderDevice.hpp"
#include "../Rendering/RenderContext2D.hpp"
#include "../Core/Input.hpp"

class Scene;

struct System
{
	System() : IsEnabled(true) {}

	bool IsEnabled;
};

struct UpdaterSystem : public System
{
	virtual void OnStart(Scene& scene) = 0;

	virtual void OnUpdate(Scene& scene, float delta, KeyboardDevice& keyboard, MouseDevice& mouse) = 0;
};

struct RendererSystem : public System
{
	virtual void OnStart(Scene& scene, RenderDevice& renderDevice) = 0;

	virtual void OnRender(Scene& scene, RenderDevice& renderDevice, RenderContext2D& renderContext2D, FrameBufferHandle target) = 0;
};

struct ScreenVertex
{
	ScreenVertex(const glm::vec2& position) : Position(position) {}

	glm::vec2 Position;

	static BufferLayout GetLayout()
	{
		BufferLayout result;
		result.AddAttribute<decltype(Position)>();
		return result;
	}
};