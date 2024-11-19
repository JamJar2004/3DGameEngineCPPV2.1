#pragma once

#include <unordered_map>

#include <ECS/Component.hpp>

#include "../Core/Input.hpp"

class KeyboardMovementComponent : public ECS::Component<KeyboardMovementComponent>
{
private:
	std::unordered_map<Key, glm::vec3> m_controls;
public:
	void SetKeyControl(Key key, const glm::vec3& direction, float speed) { m_controls[key] = glm::normalize(direction) * speed; }
	
	void RemoveKeyControl(Key key) { m_controls.erase(key); }

	auto begin() const { return m_controls.begin(); }
	auto end()   const { return m_controls.end();   }
};