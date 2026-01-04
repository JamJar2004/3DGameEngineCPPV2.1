#pragma once

#include <ECS/Component.hpp>
#include <ECS/Event.hpp>

template<glm::length_t L, std::floating_point T, typename U, glm::qualifier Q>
glm::vec<L, T, Q> Lerp(const glm::vec<L, T, Q>& left, const glm::vec<L, T, Q>& right, U factor)
{
	return glm::mix(left, right, factor);
}

template<std::floating_point T>
T Lerp(T left, T right, T factor)
{
	return left + (right - left) * factor;
}

template<typename T>
concept Animatable = requires(T obj1, T obj2, float factor)
{
	{ Lerp(obj1, obj2, factor) } -> std::same_as<T>;
};

//template<Animatable T>
//struct Frame
//{
//	Frame(T value) : Value(value) {}
//
//	const T Value;
//};
//
//template<Animatable T>
//Frame<T> Lerp(const Frame<T>& left, const Frame<T>& right, float factor) { return Lerp(left.Value, right.Value, factor); }

enum class LoopMode
{
	None, Reset, Interpolate
};

template<Animatable T>
struct Frame
{
	Frame(float timeBeforeInSeconds, const T& value) : TimeBeforeInSeconds(timeBeforeInSeconds), Value(value) {}

	float        TimeBeforeInSeconds;
	T            Value;
	ECS::Event<> OnFrameStart;
};

template<Animatable T>
class AnimationComponent : public ECS::Component<AnimationComponent<T>>
{
private:
	T& m_dest;

	std::vector<Frame<T>> m_frames;

	size_t m_currFrameIndex;

	float m_currFrameFactor;
	
	float m_totalSeconds;

	float m_elapsedSeconds;
public:
	AnimationComponent(T& dest, float loopBackTime, bool isPlaying) :
		m_dest(dest), 
		m_currFrameIndex(0),
		m_currFrameFactor(0.0f),
		m_totalSeconds(0.0f),
		m_elapsedSeconds(0.0f),
		Loop(false),
		IsPlaying(isPlaying),
		SpeedFactor(1.0f)
	{
		m_frames.emplace_back(loopBackTime, m_dest);
	}

	bool Loop;

	bool IsPlaying;

	float SpeedFactor;

	ECS::Event<>      OnReset;
	ECS::Event<float> OnElapsed;
	ECS::Event<>      OnFinish;

	void Start()   { IsPlaying = true;  }
	void Pause()   { IsPlaying = false; }
	void Restart() { Reset(); Start();  }
	void Stop()    { Pause(); Reset();  }

	float GetTotalSeconds() const { return m_totalSeconds; }

	float GetElapsedSeconds() const { return m_elapsedSeconds; }

	float GetProgress() const { return m_elapsedSeconds / m_totalSeconds; }

	void AddFrame(float secondsBefore, const T& frame)
	{
		if(secondsBefore == 0.0f)
		{
			return;
		}
		m_frames.emplace_back(secondsBefore, frame);
		m_totalSeconds += secondsBefore;
	}

		  Frame<T>& operator[](size_t index)       { return m_frames[index]; }
	const Frame<T>& operator[](size_t index) const { return m_frames[index]; }

	void RemoveFrame(size_t index) 
	{
		m_totalSeconds -= m_frames[index].first;
		m_frames.erase(m_frames.begin() + index);
	}

	void Reset()
	{
		m_currFrameIndex = 0;
		m_currFrameFactor = 0.0f;
		m_dest = m_frames[0].Value;
		m_elapsedSeconds = 0.0f;
		OnReset();
	}

	void Update(float delta)
	{
		size_t nextFrameIndex = m_currFrameIndex + 1;
		if(nextFrameIndex == m_frames.size())
		{
			if(!Loop)
			{
				OnFinish();
				m_currFrameIndex = 0;
				IsPlaying = false;
				return;
			}
			nextFrameIndex = 0;
		}

		m_dest = Lerp(m_frames[m_currFrameIndex].Value, m_frames[nextFrameIndex].Value, m_currFrameFactor);

		m_currFrameFactor += delta * SpeedFactor / m_frames[nextFrameIndex].TimeBeforeInSeconds;

		if(m_currFrameFactor >= 1.0f)
		{
			m_frames[m_currFrameIndex++].OnFrameStart();
			m_currFrameFactor = m_currFrameFactor - 1.0f;
		}

		if(m_currFrameIndex == m_frames.size() && Loop)
		{
			m_currFrameIndex = 0;
			OnReset();
		}

		OnElapsed(m_elapsedSeconds);
		m_elapsedSeconds += delta;
	}
};