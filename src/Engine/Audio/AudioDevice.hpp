#pragma once

#include "../Core/Buffer.hpp"
#include "AudioSource.hpp"



class AudioDevice
{
public:
	virtual ~AudioDevice() = default;

	virtual AudioSourceHandle CreateAudioSource() = 0;

	virtual SoundHandle CreateSound(ConstBufferSlice<SoundBalance> samples) = 0;
	//virtual MusicHandle CreateMusic(ConstBufferSlice<SoundSample> samples) = 0;

	// virtual void PlayMusic(MusicHandle music, bool loop) = 0;
	// virtual void StopMusic()  = 0;
	// virtual void PauseMusic(bool pause) = 0;

	//virtual void SetMusicPosition(float seconds) = 0;
};