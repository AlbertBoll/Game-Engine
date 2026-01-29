#pragma once
#include "Core/Utility.h"
#include "Events/Event.h"

enum class WindowFlags: uint32_t
{
    INVISIBLE = 0x1,
    FULLSCREEN = 0X2,
    BORDERLESS = 0x4,
    RESIZABLE = 0x8
};
ENABLE_BITWISE_OPERATION(WindowFlags)

enum class WindowPos: uint8_t
{
    TopLeft,
    TopRight,
    ButtomLeft,
    ButtomRight,
    Center
};

struct WindowProperties
{
    std::string m_Title = "Engine App";
    uint32_t m_Width = 800;
    uint32_t m_Height = 600;
    uint32_t m_MinWidth = 480;
    uint32_t m_MinHeight = 320;

    float m_AspectRatio = 16.f / 9.f;
    float m_Red = 0.f;
    float m_Green = 0.f;
    float m_Blue = 0.f;
    WindowFlags flag = WindowFlags::RESIZABLE ;
    WindowPos m_WinPos = WindowPos::Center;
    int m_XPaddingToCenterY = 20;
    int m_YPaddingToCenterX = 20;
    bool m_IsVsync = true;

    void SetColor(float R, float G, float B) { m_Red = R; m_Green = G; m_Blue = B; }
    void SetCornFlowerBlue() {
        m_Red = static_cast<float>(0x64)/ static_cast<float>(0xFF); 
        m_Green = static_cast<float>(0x95)/ static_cast<float>(0xFF);
        m_Blue = static_cast<float>(0xED)/ static_cast<float>(0xFF);
    }
};

class Window
{
    
public:
    using EventCallbackFn = std::function<void(Event&)>;
    Window() = default;
    virtual ~Window(){};
    uint32_t GetScreenWidth()const { return m_WindowProperties.m_Width; }
    uint32_t GetScreenHeight()const { return m_WindowProperties.m_Height; }
    virtual void Initialize(const WindowProperties& winProp = {}) = 0;
   
    WindowProperties& GetWindowProperties(){ return m_WindowProperties; }
    // Window attributes
	virtual void SetEventCallback(const EventCallbackFn& callback) { m_EventCallback = callback; }
    virtual void* GetNativeWindow() const = 0;
    virtual void ShutDown() = 0;
    virtual void OnUpdate() = 0;
    static Scoped<Window> Create(const WindowProperties& winProp = {});
    bool IsVSync()const {return m_WindowProperties.m_IsVsync;}
    virtual void SwapBuffers() = 0;
    void PollEvents(){ PumpEvents(); }

protected:
    virtual void PumpEvents() = 0;



protected:
    WindowProperties m_WindowProperties{};
    EventCallbackFn m_EventCallback;
    //uint32_t m_ScreenWidth, m_ScreenHeight;
    //float m_AspectRatio = 16.f / 9.f;
};



