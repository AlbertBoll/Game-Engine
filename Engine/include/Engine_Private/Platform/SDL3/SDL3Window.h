#pragma once
#include"Windows/Window.h"

class SDL_Window;

class SDL3Window: public Window
{
public:
    SDL3Window() = default;

    virtual ~SDL3Window();
    void Initialize(const WindowProperties& winProp) override;
    void* GetNativeWindow() const override;
    void ShutDown() override;
    void OnUpdate() override;
    uint32_t GetWindowFlag(const WindowProperties& winProp);
    void SetWindow(int DisplayWidth, int DisplayHeight, uint32_t flag);
    void SwapBuffers() override;
  
private:
    virtual void PumpEvents() override;

private:
    SDL_Window* m_Window{};
    void* m_Context{};
};