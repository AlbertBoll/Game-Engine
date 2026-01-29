#include"Windows/Window.h"
#if defined(USE_SDL3)
    #include"Platform/SDL3/SDL3Window.h"
#elif defined(USE_GLFW)
    #include "Platform/GLFW/GLFWWindow.h"
#endif

Scoped<Window> Window::Create(const WindowProperties& winProp)
{
    #if defined(USE_SDL3)
        Scoped<Window> window = CreateScopedPtr<SDL3Window>();
        window->Initialize(winProp);
        return window;
    #endif
}
