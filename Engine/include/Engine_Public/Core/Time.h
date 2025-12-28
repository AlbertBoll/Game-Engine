#pragma once
#include <algorithm>  // std::max
#include <cstdint>
    
class Time
{
public:
    // Backend-provided (defined in exactly ONE cpp: SDL3 or GLFW)
    static void   Init();            // must set baseline
    static double GetTimeSeconds();  // monotonic seconds since Init()
    static void   SleepSeconds(double seconds);

    // ---------------- FPS CAPPING (header-only) ----------------
    // fps <= 0 => uncapped
    static void SetTargetFPS(double fps)
    {
        if (fps <= 0.0) 
            s_TargetFrameSec = 0.0;
        else            
            s_TargetFrameSec = 1.0 / fps;
    }

    static double GetTargetFPS()
    {
        return (s_TargetFrameSec > 0.0) ? (1.0 / s_TargetFrameSec) : 0.0;
    }

    // dt clamp (0 => no clamp)
    static void SetMaxDeltaSeconds(double maxDt)
    {
        s_MaxDeltaSec = std::max(0.0, maxDt);
    }

    static double GetMaxDeltaSeconds()
    {
        return s_MaxDeltaSec;
    }

    // Call at top of loop. Returns dt (seconds) since last StartFrame()
    static double StartFrame()
    {
        s_ThisFrameStart = GetTimeSeconds();

        double dt = s_ThisFrameStart - s_LastFrameStart;
        s_LastFrameStart = s_ThisFrameStart;

        if (dt < 0.0) dt = 0.0; // defensive

        if (s_MaxDeltaSec > 0.0 && dt > s_MaxDeltaSec)
            dt = s_MaxDeltaSec;

        return dt;
    }

    // Call at end of loop.
    // If vsyncEnabled==true, do NOT sleep (avoid double-capping).
    static void EndFrame(bool vsyncEnabled)
    {
        if (vsyncEnabled || s_TargetFrameSec <= 0.0)
            return;

        const double frameEnd  = GetTimeSeconds();
        const double frameSec  = frameEnd - s_ThisFrameStart;
        const double remaining = s_TargetFrameSec - frameSec;

        if (remaining > 0.0)
            SleepSeconds(remaining);
    }

    // Optional convenience
    static double Now() { return GetTimeSeconds(); }

    // Let backends set the baseline to prevent first-frame dt spike
    static void SetBaselineNow()
    {
        s_LastFrameStart = GetTimeSeconds();
        s_ThisFrameStart = s_LastFrameStart;
    }

private:
    inline static double s_TargetFrameSec = 0.0;  // 0 => uncapped
    inline static double s_MaxDeltaSec    = 0.25; // default
    inline static double s_LastFrameStart = 0.0;
    inline static double s_ThisFrameStart = 0.0;
};