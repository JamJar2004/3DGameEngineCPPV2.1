#pragma once

#include <unordered_set>
#include <unordered_map>
#include <string>

#include "../../Audio/AudioSource.hpp"
#include "../../Audio/AudioDevice.hpp"

struct Mix_Chunk;
struct _Mix_Music;

//class SDLSound : public Sound
//{
//public:
//	SDLSound(Mix_Chunk* chunk) : Chunk(chunk) {}
//
//	virtual ~SDLSound();
//
//	Mix_Chunk* Chunk;
//};

class SDLWaveSound : public Sound
{
public:
	SDLWaveSound(float* buffer, size_t sampleCount) : Buffer(buffer), SampleCount(sampleCount) {}

	virtual ~SDLWaveSound();

	float* Buffer;
	size_t SampleCount;
};


class SDLMusic : public Music
{
public:
	SDLMusic(_Mix_Music* music) : Music(music) {}

	virtual ~SDLMusic();

	_Mix_Music* Music;
};

class SDLAudioDevice;

struct PlayingContext
{
	PlayingContext(float* start, size_t sampleCount) : 
		Position(start), SamplesLeft(sampleCount),
		Volume(1.0f),
		PitchSpeed(1.0f),
		LeftVolume(1.0f),
		RightVolume(1.0f) {}

	float* Position;
	size_t SamplesLeft;

	float Volume;
	float PitchSpeed;
	float LeftVolume;
	float RightVolume;
};

class SDLAudioSource : public AudioSource
{
private:
	bool m_isPlaying;

	SDLAudioDevice* m_device;

	std::shared_ptr<SDLWaveSound> m_playingSound;
public:
	SDLAudioSource(SDLAudioDevice* device);

	virtual ~SDLAudioSource();

	PlayingContext PlayingContext;

	virtual void Play(SoundHandle sound, bool loop = false) override;

	virtual void Pause() override;
	virtual void Stop() override;

	virtual void SetVolume(float volume);
	virtual void SetBalance(float leftVolume, float rightVolume);
	virtual void SetPitchSpeed(float pitchSpeed) override;

	virtual bool IsPlaying() override;

	friend class SDLAudioDevice;
};

class SDLAudioDevice : public AudioDevice
{
private:
	uint32_t m_deviceID;

	//class InstanceStorage
	//{
	//public:
	//	static SDLAudioDevice* Instance;

	//	static void OnChannelFinished(int channel);
	//};

	const size_t m_maxChannelCount;

	int m_lastChannelIndex;

	std::unordered_set<SDLAudioSource*> m_sources;

	std::shared_ptr<SDLMusic> m_playingMusic;
public:
	SDLAudioDevice(size_t maxChannelCount = 128, size_t maxSoundCount = 1024);

	virtual AudioSourceHandle CreateAudioSource() override;

	virtual SoundHandle CreateSound(const std::string& fileName) override;
	virtual MusicHandle CreateMusic(const std::string& fileName) override;

	void PlayMusic(MusicHandle music, bool loop) override;
	void StopMusic() override;
	void PauseMusic(bool pause) override;

	void SetMusicPosition(float seconds) override;

	friend class SDLAudioSource;
};

