#pragma once

#include <memory>

#include "Sound.hpp"
#include "Engine/Core/Timing.hpp"

class Music
{
public:
	virtual ~Music() = default;
};

using MusicHandle = std::shared_ptr<Music>;


class AudioSourceState
{
public:
	AudioSourceState() :
		Volume(1.0),
		PitchSpeed(1.0),
		IsPlaying(false),
		IsLoopEnabled(false) {}

	float Volume;
	float PitchSpeed;

	SoundBalance Balance {};

	bool IsPlaying;
	bool IsLoopEnabled;
};

class AudioSource
{
public:
	explicit AudioSource(size_t sampleRate) : SampleRate(sampleRate), m_sampleIndex(0) {}

	virtual ~AudioSource() = default;

	const size_t SampleRate;

	AudioSourceState State;

	[[nodiscard]] const Sound* GetCurrentSound() const { return m_currentSound.get(); }

	[[nodiscard]] Duration GetAbsolutePosition() const
	{
		return Duration::FromSeconds(static_cast<double>(m_sampleIndex) / static_cast<double>(SampleRate));
	}

	void SetAbsolutePosition(Duration position)
	{
		m_sampleIndex = static_cast<size_t>(position.TotalSeconds() * static_cast<double>(SampleRate));
	}

	[[nodiscard]] double GetPosition() const
	{
		return static_cast<double>(m_sampleIndex) / static_cast<double>(m_currentSound->GetSamples().Count());
	}

	void SetPosition(double position)
	{
		m_sampleIndex = static_cast<size_t>(position * static_cast<double>(m_currentSound->GetSamples().Count()));
	}

	[[nodiscard]] size_t GetSampleIndex() const { return m_sampleIndex; }

	void Play(const SoundHandle& sound)
	{
		Reset();
		m_currentSound = sound;
		State.IsPlaying = true;
	}

	void Reset()
	{
		m_sampleIndex = 0;
	}

	void Replay()
	{
		Reset();
		State.IsPlaying = true;
	}

	void Resume()
	{
		if (m_currentSound)
		{
			if (m_sampleIndex < m_currentSound->GetSamples().Count())
			{
				State.IsPlaying = true;
			}
		}
	}

	void Pause()
	{
		State.IsPlaying = false;
	}

	void Stop()
	{
		State.IsPlaying = false;
		Reset();
	}
protected:
	void MoveSamples(size_t offset) { m_sampleIndex += offset; }
private:
	SoundHandle m_currentSound;

	size_t m_sampleIndex;
};

using AudioSourceHandle = std::shared_ptr<AudioSource>;