#include"Input/Input.h"
#include"Input/InputBackend.h"
#include"Windows/Window.h"




void Input::Init(Window& window) {
    InputBackend_Init(window);
}

void Input::Shutdown() {
    InputBackend_Shutdown();
}

void Input::BeginFrame() 
{
    std::copy(std::begin(s_CurKeys),  std::end(s_CurKeys),  std::begin(s_PrevKeys));
    std::copy(std::begin(s_CurMouse), std::end(s_CurMouse), std::begin(s_PrevMouse));
    s_MouseDX = s_MouseDY = 0.0f;

    for (size_t i = 0; i < KeyCount; i++)
        s_CurKeys[i] = InputBackend_GetKeyDown(static_cast<Key>(i));

    for (int i = 0; i < (int)MouseButton::Count; i++)
        s_CurMouse[i] = InputBackend_GetMouseDown(static_cast<MouseButton>(i));

    float x, y;
    InputBackend_GetMousePos(x, y);
    s_MouseX = x; s_MouseY = y;

    s_MouseDX = s_MouseX - s_LastMouseX;
    s_MouseDY = s_MouseY - s_LastMouseY;
    s_LastMouseX = s_MouseX;
    s_LastMouseY = s_MouseY;
}

bool Input::IsKeyDown(Key k)      { return s_CurKeys[(size_t)k]; }
bool Input::WasKeyPressed(Key k)  { return s_CurKeys[(size_t)k] && !s_PrevKeys[(size_t)k]; }
bool Input::WasKeyReleased(Key k) { return !s_CurKeys[(size_t)k] && s_PrevKeys[(size_t)k]; }
bool Input::IsKeyHeld(Key k)
{
    return s_CurKeys[(size_t)k] && s_PrevKeys[(size_t)k];
}

bool Input::IsMouseDown(MouseButton b)     { return s_CurMouse[(size_t)b]; }
bool Input::WasMousePressed(MouseButton b) { return s_CurMouse[(size_t)b] && !s_PrevMouse[(size_t)b]; }
bool Input::IsMouseHeld(MouseButton b)
{
    return s_CurMouse[(size_t)b] && s_PrevMouse[(size_t)b];
}

float Input::MouseX() { return s_MouseX; }
float Input::MouseY() { return s_MouseY; }
float Input::MouseDX(){ return s_MouseDX; }
float Input::MouseDY() { return s_MouseDY; }

    