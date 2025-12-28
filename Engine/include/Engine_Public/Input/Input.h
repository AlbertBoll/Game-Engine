#pragma once

#include"Codes.h"
#include<cstddef>

class Window;

class Input
{
public:
    static void Init(Window& window);
    static void Shutdown();
    static void BeginFrame();

    static bool IsKeyDown(Key k);
    static bool WasKeyPressed(Key k);
    static bool WasKeyReleased(Key k);

    static bool IsMouseDown(MouseButton b);
    static bool WasMousePressed(MouseButton b);

    static float MouseX();
    static float MouseY();
    static float MouseDX();
    static float MouseDY();

private:
    inline static constexpr size_t KeyCount = 512;

    inline static bool s_CurKeys[KeyCount]{};
    inline static bool s_PrevKeys[KeyCount]{};

    inline static bool s_CurMouse[8]{};
    inline static bool s_PrevMouse[8]{};

    inline static float s_MouseX{}, s_MouseY{};
    inline static float s_LastMouseX{}, s_LastMouseY{};
    inline static float s_MouseDX{}, s_MouseDY{};
};