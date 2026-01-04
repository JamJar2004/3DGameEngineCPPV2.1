#pragma once

#include <ECS/Component.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

//template<typename T>
//class Observable
//{
//private:
//	T m_value;
//public:
//	Observable(const T& value) : m_value(value) {}
//
//	ECS::Event<T, T> ValueChanged;
//
//	operator T() const { return m_value; }
//
//	Observable<T>& operator=(const Observable<T>& source)
//	{
//		T oldValue = m_value;
//		m_value = source.m_value;
//		ValueChanged(oldValue, m_value);
//	}
//
//	Observable<T>& operator=(const T& newValue)
//	{
//		T oldValue = m_value;
//		m_value = source;
//		ValueChanged(oldValue, newValue);
//	}
//};

struct Transformation : public ECS::Component<Transformation>
{
private:
	Transformation* m_parent;
public:
	Transformation(const glm::vec3& position = glm::vec3(0, 0, 0),
		const glm::quat& rotation = glm::quat(1, 0, 0, 0), const glm::vec3& scale = glm::vec3(1, 1, 1)) :
		m_parent(nullptr), Position(position), Rotation(rotation), Scale(scale) {}

	void SetParent(ECS::Entity parentEntity)
	{
		if(parentEntity.ContainsComponent<Transformation>())
		{
			m_parent = &parentEntity.GetComponent<Transformation>();
		}
	}

	void RemoveParent() { m_parent = nullptr; }

	glm::vec3 Position;
	glm::quat Rotation;
	glm::vec3 Scale;

	glm::mat4 ToMatrix() const
	{
		glm::mat4 positionMatrix = glm::translate(glm::identity<glm::mat4>(), Position);
		glm::mat4 rotationMatrix = glm::toMat4(Rotation);
		glm::mat4    scaleMatrix = glm::scale(glm::identity<glm::mat4>(), Scale);

		glm::mat4 localMatrix = positionMatrix * rotationMatrix * scaleMatrix;

		if(m_parent)
		{
			glm::mat4 parentMatrix = m_parent->ToMatrix();
			return parentMatrix * localMatrix;
		}

		return localMatrix;
	}

	glm::vec3 GetTransformedPosition() const
	{
		if(m_parent)
		{
			return m_parent->GetTransformedPosition() + Position;
		}
		return Position;
	}

	glm::quat GetTransformedRotation() const
	{
		if(m_parent)
		{
			return m_parent->GetTransformedRotation() * Rotation;
		}
		return Rotation;
	}

	glm::vec3 GetTransformedScale() const
	{
		if(m_parent)
		{
			return m_parent->GetTransformedScale() * Scale;
		}
		return Scale;
	}

	void Rotate(const glm::quat& rotation) { Rotation = glm::normalize(rotation * Rotation); }

	void Rotate(const glm::vec3& axis, float angle) { Rotate(glm::angleAxis(glm::radians(angle), glm::normalize(axis))); }
};

inline Transformation Lerp(const Transformation& left, const Transformation& right, float factor)
{
	return Transformation(
		glm::mix (left.Position, right.Position, factor),
		glm::lerp(left.Rotation, right.Rotation, factor),
		glm::mix (left.Scale   , right.Scale   , factor));
}