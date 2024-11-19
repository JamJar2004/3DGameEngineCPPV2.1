#pragma once

#include <string>

#include "AudioSource.hpp"

class AudioDevice
{
public:
	AudioDevice() {}

	virtual ~AudioDevice() {}

	virtual AudioSourceHandle CreateAudioSource() = 0;

	virtual SoundHandle CreateSound(const std::string& fileName) = 0;
	virtual MusicHandle CreateMusic(const std::string& fileName) = 0;

	virtual void PlayMusic(MusicHandle music, bool loop) = 0;
	virtual void StopMusic()  = 0;
	virtual void PauseMusic(bool pause) = 0;

	virtual void SetMusicPosition(float seconds) = 0;
};