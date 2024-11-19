#include "SDLAudio.hpp"

#define MODPLUG_STATIC

#include <SDL2/SDL_mixer.h>
#include <iostream>

struct Sample
{
    float Left;
    float Right;
};

static void AudioCallBack(void* userData, uint8_t* buffer, int32_t length)
{
    std::unordered_set<SDLAudioSource*>& sources = *(std::unordered_set<SDLAudioSource*>*)userData;

    Sample* samples = (Sample*)buffer;
    size_t sampleCount = length / sizeof(Sample);

    memset(samples, 0, length);

    for(SDLAudioSource* source : sources)
    {
        if(source->PlayingContext.SamplesLeft == 0 || !source->PlayingContext.Position)
        {
            continue;
        }

        sampleCount = std::min(sampleCount, source->PlayingContext.SamplesLeft);

        for(size_t i = 0; i < sampleCount; i++)
        {
            PlayingContext& context = source->PlayingContext;

            float amplitude = context.Position[i] * context.Volume;

            samples[i].Left  += amplitude * context.LeftVolume;
            samples[i].Right += amplitude * context.RightVolume;
        }

        source->PlayingContext.Position    += sampleCount;
        source->PlayingContext.SamplesLeft -= sampleCount;
    }
}

SDLAudioDevice::SDLAudioDevice(size_t maxChannelCount, size_t maxSoundCount) : m_maxChannelCount(maxChannelCount), m_lastChannelIndex(0)
{
    //InstanceStorage::Instance = this;

    SDL_AudioSpec desiredSpec = {};
    desiredSpec.freq     = 44100;
    desiredSpec.format   = AUDIO_F32;
    desiredSpec.channels = 2;
    desiredSpec.samples  = 2048; // Adjust buffer size
    desiredSpec.callback = AudioCallBack;
    desiredSpec.userdata = &m_sources;

    m_deviceID = SDL_OpenAudioDevice(nullptr, 0, &desiredSpec, nullptr, SDL_AUDIO_ALLOW_ANY_CHANGE);

    SDL_PauseAudioDevice(m_deviceID, 0);

    if(Mix_OpenAudio(48000, AUDIO_S16, 2, 1024) == -1)
    {
        printf(Mix_GetError());
    }
    
    Mix_AllocateChannels(maxChannelCount);

    //Mix_ChannelFinished(InstanceStorage::OnChannelFinished);
}

//SDLAudioDevice* SDLAudioDevice::InstanceStorage::Instance = nullptr;

//void SDLAudioDevice::InstanceStorage::OnChannelFinished(int channel)
//{
//    Instance->m_sources[channel]->m_playingSound = nullptr;
//}

AudioSourceHandle SDLAudioDevice::CreateAudioSource()
{
    auto result = std::make_shared<SDLAudioSource>(this);
    m_sources.insert(result.get());
    return result;
}

SoundHandle SDLAudioDevice::CreateSound(const std::string& fileName)
{
    SDL_AudioSpec spec;
    uint8_t* buffer;
    uint32_t length;

    if(SDL_LoadWAV(fileName.c_str(), &spec, &buffer, &length) == 0)
    {
        printf(SDL_GetError());
    }

    float* targetBuffer = nullptr;
    size_t sampleCount = length / sizeof(int16_t) / spec.channels;

    //switch(spec.format)
    //{
    //    case AUDIO_S8:
    //        sampleCount = length / sizeof(int8_t);
    //        break;
    //    case AUDIO_S16:
    //        sampleCount = length / sizeof(int16_t);
    //        break;
    //    case AUDIO_S32:
    //        sampleCount = length / sizeof(int32_t);
    //        break;
    //    case AUDIO_F32:
    //        sampleCount = length / sizeof(float);
    //        break;
    //    default:
    //        break;
    //}

    int16_t* const sourceBuffer = (int16_t*)buffer;
    int16_t* sample = sourceBuffer;

    targetBuffer = new float[sampleCount];
    
    for(size_t i = 0; i < sampleCount; i++, sample += spec.channels)
    {
        targetBuffer[i] = 0.0f;
        for(size_t j = 0; j < spec.channels; j++)
        {
            targetBuffer[i] += sample[j] / float(spec.channels) / 32767.0f;
        }
    }

    return std::make_shared<SDLWaveSound>(targetBuffer, sampleCount);
}

//SoundHandle SDLAudioDevice::CreateSound(const std::string& fileName)
//{
//    Mix_Chunk* chunk = Mix_LoadWAV(fileName.c_str());
//    printf(Mix_GetError());
//    return std::make_shared<SDLWaveSound>(chunk);
//}

MusicHandle SDLAudioDevice::CreateMusic(const std::string& fileName)
{
    Mix_Music* music = Mix_LoadMUS(fileName.c_str());
    printf(Mix_GetError());
    return std::make_shared<SDLMusic>(music);
}

void SDLAudioDevice::PlayMusic(MusicHandle music, bool loop)
{
    m_playingMusic = std::dynamic_pointer_cast<SDLMusic>(music);
    Mix_PlayMusic(m_playingMusic->Music, loop ? 1 : 0);
}

void SDLAudioDevice::StopMusic()
{
    Mix_PauseMusic();
    Mix_RewindMusic();
}

void SDLAudioDevice::PauseMusic(bool pause)
{
    if(pause)
    {
        Mix_PauseMusic();
    }
    else
    {
        Mix_ResumeMusic();
    }
}

void SDLAudioDevice::SetMusicPosition(float seconds)
{
    Mix_SetMusicPosition(seconds);
    printf(Mix_GetError());
}

SDLAudioSource::SDLAudioSource(SDLAudioDevice* device) : m_device(device), PlayingContext(nullptr, 0)
{
    
}

SDLAudioSource::~SDLAudioSource()
{
    m_device->m_sources.erase(this);
}

void SDLAudioSource::Play(SoundHandle sound, bool loop)
{
    auto sdlSound = std::dynamic_pointer_cast<SDLWaveSound>(sound);
    m_playingSound = sdlSound;

    PlayingContext.Position = sdlSound->Buffer;
    PlayingContext.SamplesLeft = sdlSound->SampleCount;
    //Mix_PlayChannel(ChannelIndex, chunk, loop ? -1 : 0);
}

void SDLAudioSource::Pause()
{
    m_isPlaying = false;
}

void SDLAudioSource::Stop()
{
    m_isPlaying = false;

    PlayingContext.Position    = m_playingSound->Buffer;
    PlayingContext.SamplesLeft = m_playingSound->SampleCount;
}

void SDLAudioSource::SetVolume(float volume)
{
    PlayingContext.Volume = volume;
}

void SDLAudioSource::SetBalance(float leftVolume, float rightVolume)
{
    PlayingContext.LeftVolume  = leftVolume;
    PlayingContext.RightVolume = rightVolume;
}

void SDLAudioSource::SetPitchSpeed(float pitchSpeed)
{
    PlayingContext.PitchSpeed = pitchSpeed;
}

bool SDLAudioSource::IsPlaying()
{
    return m_isPlaying;
}

SDLWaveSound::~SDLWaveSound()
{
    delete[] Buffer;
    //Mix_FreeChunk(Chunk);
}

//SDLSound::~SDLSound()
//{
//    Mix_FreeChunk(Chunk);
//}

SDLMusic::~SDLMusic()
{
    Mix_FreeMusic(Music);
}
