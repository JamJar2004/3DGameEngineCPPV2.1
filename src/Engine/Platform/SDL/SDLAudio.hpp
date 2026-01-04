#pragma once

#include <unordered_set>

#include "../../Audio/AudioSource.hpp"
#include "../../Audio/AudioDevice.hpp"
#include "Engine/Core/Buffer.hpp"

//struct Mix_Chunk;
//struct _Mix_Music;

//class SDLSound : public Sound
//{
//public:
//	SDLSound(Mix_Chunk* chunk) : Chunk(chunk) {}
//
//	virtual ~SDLSound();
//
//	Mix_Chunk* Chunk;
//};

class SDLSound final : public Sound
{
public:
	explicit SDLSound(ConstBufferSlice<SoundBalance> buffer) : m_buffer(buffer) {}

	BufferSlice<SoundBalance> GetSamples() override { return m_buffer; }

	[[nodiscard]] ConstBufferSlice<SoundBalance> GetSamples() const override { return m_buffer; }
private:
	Buffer<SoundBalance> m_buffer;
};


// class SDLMusic : public Music
// {
// public:
// 	SDLMusic(_Mix_Music* music) : Music(music) {}
//
// 	virtual ~SDLMusic();
//
// 	_Mix_Music* Music;
// };

class SDLAudioSource;

class SDLAudioDevice final : public AudioDevice, std::enable_shared_from_this<SDLAudioDevice>
{
public:
	explicit SDLAudioDevice(size_t sampleRate);

	const size_t SampleRate;

	AudioSourceHandle CreateAudioSource() override;

	SoundHandle CreateSound(ConstBufferSlice<SoundBalance> samples) override;
	//MusicHandle CreateMusic(ConstBufferSlice<float> samples) override;

	// void PlayMusic(MusicHandle music, bool loop) override;
	// void StopMusic() override;
	// void PauseMusic(bool pause) override;

	//void SetMusicPosition(float seconds) override;

	friend class SDLAudioSource;
private:
	std::unordered_set<SDLAudioSource*> m_activeSources;

	uint32_t m_deviceID;
};

class SDLAudioSource final : public AudioSource
{
public:
	explicit SDLAudioSource(const std::shared_ptr<SDLAudioDevice>& device);

	~SDLAudioSource() override;

	friend class SDLAudioDevice;
private:
	std::shared_ptr<SDLAudioDevice> m_device;
};



