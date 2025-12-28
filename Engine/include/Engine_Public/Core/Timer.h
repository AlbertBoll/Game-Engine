#pragma once
#include <chrono>

class Timer
{
public:
    Timer()
    {
        Reset();
    }

    void Reset()
    {
        m_Start = std::chrono::steady_clock::now();
    }

    float Elapsed()
    {
        using namespace std::chrono;
        return duration<float>(steady_clock::now() - m_Start).count(); // seconds
        //return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_Start).count() * 0.001f * 0.001f * 0.001f;
    }

    float ElapsedMillis()
    {
        return Elapsed() * 1000.0f;
    }

private:
    std::chrono::steady_clock::time_point m_Start;
};