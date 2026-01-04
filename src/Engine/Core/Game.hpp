#pragma once

#include <string>
#include <chrono>
#include <utility>

#include "Application.hpp"
#include "Scene.hpp"

#include "../Rendering/UserInterface/DefaultStyle.hpp"

class GameSettings
{
public:
	GameSettings(ScreenGraphicsMode graphicsMode, std::string title, SceneHandle  startScene, double frameRate = 60.0) :
		GraphicsMode(graphicsMode),
		Title(std::move(title)),
		StartScene(std::move(startScene)),
		FrameRate(frameRate) {}

	ScreenGraphicsMode GraphicsMode;
	std::string        Title;
	SceneHandle        StartScene;
	double             FrameRate;
};

class Timer
{
public:
	static const std::vector<Timer*>& GetTimers() { return s_timers; }

	explicit Timer(std::string_view name) :
		Name(name), m_sampleCount(0), m_durationSamples(Duration::Zero())
	{
		s_timers.push_back(this);
	}

	void AddSample(Duration sample)
	{
		++m_sampleCount;
		m_durationSamples += sample;
	}

	Duration GetSamplesAverage()
	{
		if(m_sampleCount == 0)
		{
			return Duration::Zero();
		}

		const auto result = m_durationSamples / static_cast<double>(m_sampleCount);
		m_sampleCount = 0;
		m_durationSamples = Duration::Zero();
		return result;
	}

	std::string_view Name;
private:
	static std::vector<Timer*> s_timers;

	size_t m_sampleCount;
	Duration m_durationSamples;
};


class ScopeTimer
{
public:
	explicit ScopeTimer(Timer& timer) : SourceTimer(timer), m_startTime(Clock::CurrentTime()) {}

	~ScopeTimer()
	{
		const auto stopTime = Clock::CurrentTime();
		SourceTimer.AddSample(stopTime - m_startTime);
	}

	Timer& SourceTimer;
private:
	TimeStamp m_startTime;
};

GameSettings StartSettings();

struct Game
{
	explicit Game() : Settings(StartSettings()) {}

	GameSettings Settings;

	void Start(Application& app) const;
};