#include "Core/Time.h"
#include <SDL3/SDL.h>
#include <thread>
#include <chrono>

static uint64_t s_StartCounter = 0;
static double   s_Frequency    = 1.0;

void Time::Init()
{
    s_StartCounter = SDL_GetPerformanceCounter();
    s_Frequency = static_cast<double>(SDL_GetPerformanceFrequency());
    if (s_Frequency <= 0.0) 
        s_Frequency = 1.0;

    SetBaselineNow();
}

double Time::GetTimeSeconds()
{
    const uint64_t now = SDL_GetPerformanceCounter();
    return static_cast<double>(now - s_StartCounter) / s_Frequency;
}

void Time::SleepSeconds(double seconds)
{
    if (seconds <= 0.0) return;
    // rely on std::sleep_for
    // use SDL_Delay for ms, but this is fine and portable.)
    std::this_thread::sleep_for(std::chrono::duration<double>(seconds));
    
}