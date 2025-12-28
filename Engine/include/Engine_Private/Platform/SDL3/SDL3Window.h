#pragma once
#include"Windows/Window.h"
//#include <SDL3/SDL.h>

class SDL_Window;

class SDL3Window: public Window
{
public:
    SDL3Window() = default;
    void Initialize(const WindowProperties& winProp) override;
    void* GetNativeWindow() const override;
    void ShutDown() override;


private:
     SDL_Window* m_Window{};
};