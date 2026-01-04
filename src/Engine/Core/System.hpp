#pragma once

#include "../Core/Input.hpp"
#include "../Rendering/RenderContext2D.hpp"

class Scene;

struct System
{
	virtual ~System() = default;

	System() : IsEnabled(true) {}

	bool IsEnabled;
};

struct UpdaterSystem : System
{
	virtual void OnUpdate(Scene& scene, float delta, KeyboardDevice& keyboard, MouseDevice& mouse) = 0;
};

struct RendererSystem : System
{
	virtual void OnRender(Scene& scene, RenderDevice& renderDevice, RenderContext2D& renderContext2D, RenderTarget& target) = 0;
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