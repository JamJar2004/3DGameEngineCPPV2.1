#pragma once

#include <memory>

class Sound
{
public:
	virtual ~Sound() {}
};

using SoundHandle = std::shared_ptr<Sound>;

class Music
{
public:
	virtual ~Music() {}
};

using MusicHandle = std::shared_ptr<Music>;

class AudioSource
{
public:
	virtual ~AudioSource() {}

	virtual void Play(SoundHandle sound, bool loop = false) = 0;
	virtual void Pause() = 0;
	virtual void Stop() = 0;

	virtual void SetVolume(float volume) = 0;
	virtual void SetBalance(float leftVolume, float rightVolume) = 0;
	virtual void SetPitchSpeed(float pitchSpeed) = 0;

	virtual bool IsPlaying() = 0;
};

using AudioSourceHandle = std::shared_ptr<AudioSource>;