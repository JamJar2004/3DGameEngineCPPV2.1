#pragma once

#include <string>
#include <chrono>

#include "Application.hpp"
#include "Scene.hpp"

#include "../Rendering/UserInterface/DefaultStyle.hpp"

class GameSettings
{
public:
	GameSettings(GraphicsMode graphicsMode, const std::string& title, SceneHandle startScene, double frameRate = 60.0) :
		GraphicsMode(graphicsMode), Title(title), StartScene(startScene), FrameRate(frameRate) {}

	GraphicsMode GraphicsMode;
	std::string  Title;
	SceneHandle  StartScene;
	double       FrameRate;
};

struct Timer
{
private:
	static std::vector<Timer*> s_timers;

	size_t m_sampleCount;
	std::chrono::system_clock::duration m_durationSamples;

public:
	static const std::vector<Timer*>& GetTimers() { return s_timers; }

	Timer(std::string_view name) :
		Name(name), m_sampleCount(0), m_durationSamples(std::chrono::system_clock::duration::zero()) 
	{
		s_timers.push_back(this);
	}



	void AddSample(std::chrono::system_clock::duration sample)
	{
		++m_sampleCount;
		m_durationSamples += sample;
	}

	std::chrono::duration<double, std::milli> GetSamplesAverage()
	{
		if(m_sampleCount == 0)
		{
			return std::chrono::milliseconds::zero();
		}

		auto result = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(m_durationSamples / m_sampleCount);
		m_sampleCount = 0;
		m_durationSamples = std::chrono::system_clock::duration::zero();
		return result;
	}

	std::string_view Name;
};


class ScopeTimer
{
private:
	std::chrono::system_clock::time_point m_startTime;
public:
	ScopeTimer(Timer& timer) : Timer(timer)
	{
		m_startTime = std::chrono::system_clock::now();
	}

	~ScopeTimer()
	{
		auto stopTime = std::chrono::system_clock::now();
		Timer.AddSample(stopTime - m_startTime);
	}

	Timer& Timer;
};


class Game
{
private:
	
public:
	GameSettings StartSettings();

	void Start(const GameSettings& settings, Application& app);
};