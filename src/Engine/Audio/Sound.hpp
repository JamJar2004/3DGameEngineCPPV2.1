#pragma once

#include <memory>

#include "Engine/Core/Buffer.hpp"

struct SoundBalance
{
	SoundBalance() : LeftVolume(1.0), RightVolume(1.0) {}

	explicit SoundBalance(const glm::vec2& volume) : SoundBalance(volume.x, volume.y) {}

	SoundBalance(float leftVolume, float rightVolume) :
		LeftVolume(leftVolume), RightVolume(rightVolume) {}

	float LeftVolume;
	float RightVolume;

	[[nodiscard]] glm::vec2 ToVector() const { return {LeftVolume, RightVolume}; }

	SoundBalance& operator+=(const SoundBalance& right)
	{
		 LeftVolume += right.LeftVolume;
		RightVolume += right.RightVolume;
		return *this;
	}

	SoundBalance& operator*=(const SoundBalance& right)
	{
		 LeftVolume *= right.LeftVolume;
		RightVolume *= right.RightVolume;
		return *this;
	}

	SoundBalance& operator+=(float right)
	{
		 LeftVolume += right;
		RightVolume += right;
		return *this;
	}

	SoundBalance& operator*=(float right)
	{
		 LeftVolume *= right;
		RightVolume *= right;
		return *this;
	}

	friend SoundBalance operator+(const SoundBalance& left, const SoundBalance& right)
	{
		return SoundBalance(left.ToVector() + right.ToVector());
	}

	friend SoundBalance operator*(const SoundBalance& left, const SoundBalance& right)
	{
		return SoundBalance(left.ToVector()* right.ToVector());
	}

	friend SoundBalance operator+(const SoundBalance& left, float right)
	{
		return SoundBalance(left.ToVector() + right);
	}

	friend SoundBalance operator*(const SoundBalance& left, float right)
	{
		return SoundBalance(left.ToVector() * right);
	}
};

class Sound
{
public:
	              virtual      BufferSlice<SoundBalance> GetSamples()       = 0;
	[[nodiscard]] virtual ConstBufferSlice<SoundBalance> GetSamples() const = 0;

	virtual ~Sound() = default;
};

using SoundHandle = std::shared_ptr<Sound>;
