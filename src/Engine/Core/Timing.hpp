#pragma once

#include <chrono>

#include "Timing.hpp"



class Duration
{
public:
    static constexpr Duration FromNanoseconds(double nanoSeconds)
    {
        return Duration(Nano(static_cast<int64_t>(nanoSeconds)));
    }

    static constexpr Duration FromMilliseconds(double milliSeconds)
    {
        return Duration(std::chrono::duration_cast<Nano>(std::chrono::duration<double, std::milli>(milliSeconds)));
    }

    static constexpr Duration FromSeconds(double seconds)
    {
        return Duration(std::chrono::duration_cast<Nano>(std::chrono::duration<double>(seconds)));
    }

    static constexpr Duration FromMinutes(double minutes)
    {
        return FromSeconds(minutes * 60.0);
    }

    static constexpr Duration FromHours(double hours)
    {
        return FromMinutes(hours * 60.0);
    }

    static constexpr Duration FromDays(double days)
    {
        return FromHours(days * 24.0);
    }

    static constexpr Duration FromWeeks(double weeks)
    {
        return FromDays(weeks * 24.0);
    }

    static constexpr Duration Zero() { return Duration(std::chrono::nanoseconds::zero()); }

    [[nodiscard]] double TotalNanoSeconds() const { return static_cast<double>(m_duration.count()); }

    [[nodiscard]] double TotalMicroSeconds() const { return std::chrono::duration<double, std::micro>(m_duration).count(); }
    [[nodiscard]] double TotalMilliSeconds() const { return std::chrono::duration<double, std::milli>(m_duration).count(); }
    [[nodiscard]] double TotalSeconds()      const { return std::chrono::duration<double>(m_duration).count(); }

    [[nodiscard]] double TotalMinutes() const { return TotalSeconds() / 60.0; }
    [[nodiscard]] double TotalHours()   const { return TotalMinutes() / 60.0; }
    [[nodiscard]] double TotalDays()    const { return TotalHours()   / 24.0; }
    [[nodiscard]] double TotalWeeks()   const { return TotalDays()    /  7.0; }

    Duration& operator+=(Duration other)
    {
        m_duration += other.m_duration;
        return *this;
    }

    Duration& operator-=(Duration other)
    {
        m_duration -= other.m_duration;
        return *this;
    }

    friend constexpr Duration operator+(Duration left, Duration right) { return Duration(left.m_duration + right.m_duration); }
    friend constexpr Duration operator-(Duration left, Duration right) { return Duration(left.m_duration + right.m_duration); }

    friend Duration operator*(Duration left, double right)
    {
        return Duration(Nano(static_cast<int64_t>(static_cast<double>(left.m_duration.count()) * right)));
    }

    friend Duration operator/(Duration left, double right)
    {
        return Duration(Nano(static_cast<int64_t>(static_cast<double>(left.m_duration.count()) / right)));
    }

    friend std::strong_ordering operator<=>(Duration left, Duration right) { return left.m_duration <=> right.m_duration; }

    friend class TimeStamp;
private:
    using Nano = std::chrono::nanoseconds;

    constexpr explicit Duration(std::chrono::nanoseconds nanoSeconds) : m_duration(nanoSeconds) {}

    std::chrono::nanoseconds m_duration;
};

class TimeStamp
{
public:
    friend TimeStamp operator+(TimeStamp left, Duration right) { return TimeStamp(left.m_stamp + right.m_duration); }
    friend TimeStamp operator-(TimeStamp left, Duration right) { return TimeStamp(left.m_stamp - right.m_duration); }

    TimeStamp& operator+=(Duration other) { m_stamp += other.m_duration; return *this; }
    TimeStamp& operator-=(Duration other) { m_stamp -= other.m_duration; return *this; }

    friend Duration operator-(TimeStamp left, TimeStamp right)
    {
        return Duration(left.m_stamp - right.m_stamp);
    }

    friend struct Clock;
private:
    using Stamp = std::chrono::high_resolution_clock::time_point;

    explicit TimeStamp(Stamp stamp) : m_stamp(stamp) {}

    Stamp m_stamp;
};

struct Clock
{
    static TimeStamp CurrentTime() { return TimeStamp(std::chrono::high_resolution_clock::now()); }
};