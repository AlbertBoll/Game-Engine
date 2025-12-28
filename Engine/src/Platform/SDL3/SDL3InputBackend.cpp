#include "Input/Codes.h"
#include "Input/InputBackend.h"
#include "Windows/Window.h"
#include <SDL3/SDL.h>

static SDL_Window* s_Window = nullptr;

// mapping helpers (only in this file)
static SDL_Scancode ToSDLScancode(Key k);       // SDL_SCANCODE_UNKNOWN if unmapped
static uint32_t ToSDLMouseMask(MouseButton b);


void InputBackend_Init(Window& window)
{
    s_Window = (SDL_Window*)window.GetNativeWindow();
}

void InputBackend_Shutdown()
 {
    s_Window = nullptr;
}

bool InputBackend_GetKeyDown(Key k)
 {
    const bool* state = (const bool*)SDL_GetKeyboardState(nullptr);
    SDL_Scancode sc = ToSDLScancode(k);
    return (sc != SDL_SCANCODE_UNKNOWN) && state[(int)sc];
}

bool InputBackend_GetMouseDown(MouseButton b)
 {
    float x, y;
    uint32_t buttons = SDL_GetMouseState(&x, &y);
    return (buttons & ToSDLMouseMask(b)) != 0;
}

void InputBackend_GetMousePos(float& x, float& y) 
{
    SDL_GetMouseState(&x, &y);
}

// SDL_Scancode ToSDLScancode(Key k)
// {
//     switch (sc)
//     {
//         case SDL_SCANCODE_SPACE:        return (int)Key::Space;
//         case SDL_SCANCODE_APOSTROPHE:   return (int)Key::Apostrophe;
//         case SDL_SCANCODE_COMMA:        return (int)Key::Comma;
//         case SDL_SCANCODE_MINUS:        return (int)Key::Minus;
//         case SDL_SCANCODE_PERIOD:       return (int)Key::Period;
//         case SDL_SCANCODE_SLASH:        return (int)Key::Slash;

//         case SDL_SCANCODE_0:            return (int)Key::D0;
//         case SDL_SCANCODE_1:            return (int)Key::D1;
//         case SDL_SCANCODE_2:            return (int)Key::D2;
//         case SDL_SCANCODE_3:            return (int)Key::D3;
//         case SDL_SCANCODE_4:            return (int)Key::D4;
//         case SDL_SCANCODE_5:            return (int)Key::D5;
//         case SDL_SCANCODE_6:            return (int)Key::D6;
//         case SDL_SCANCODE_7:            return (int)Key::D7;
//         case SDL_SCANCODE_8:            return (int)Key::D8;
//         case SDL_SCANCODE_9:            return (int)Key::D9;

//         case SDL_SCANCODE_A:            return (int)Key::A;
//         case SDL_SCANCODE_B:            return (int)Key::B;
//         case SDL_SCANCODE_C:            return (int)Key::C;
//         case SDL_SCANCODE_D:            return (int)Key::D;
//         case SDL_SCANCODE_E:            return (int)Key::E;
//         case SDL_SCANCODE_F:            return (int)Key::F;
//         case SDL_SCANCODE_G:            return (int)Key::G;
//         case SDL_SCANCODE_H:            return (int)Key::H;
//         case SDL_SCANCODE_I:            return (int)Key::I;
//         case SDL_SCANCODE_J:            return (int)Key::J;
//         case SDL_SCANCODE_K:            return (int)Key::K;
//         case SDL_SCANCODE_L:            return (int)Key::L;
//         case SDL_SCANCODE_M:            return (int)Key::M;
//         case SDL_SCANCODE_N:            return (int)Key::N;
//         case SDL_SCANCODE_O:            return (int)Key::O;
//         case SDL_SCANCODE_P:            return (int)Key::P;
//         case SDL_SCANCODE_Q:            return (int)Key::Q;
//         case SDL_SCANCODE_R:            return (int)Key::R;
//         case SDL_SCANCODE_S:            return (int)Key::S;
//         case SDL_SCANCODE_T:            return (int)Key::T;
//         case SDL_SCANCODE_U:            return (int)Key::U;
//         case SDL_SCANCODE_V:            return (int)Key::V;
//         case SDL_SCANCODE_W:            return (int)Key::W;
//         case SDL_SCANCODE_X:            return (int)Key::X;
//         case SDL_SCANCODE_Y:            return (int)Key::Y;
//         case SDL_SCANCODE_Z:            return (int)Key::Z;

//         case SDL_SCANCODE_ESCAPE:       return (int)Key::Escape;
//         case SDL_SCANCODE_RETURN:       return (int)Key::Enter;
//         case SDL_SCANCODE_TAB:          return (int)Key::Tab;
//         case SDL_SCANCODE_BACKSPACE:    return (int)Key::Backspace;
//         case SDL_SCANCODE_INSERT:       return (int)Key::Insert;
//         case SDL_SCANCODE_DELETE:       return (int)Key::Delete;
//         case SDL_SCANCODE_RIGHT:        return (int)Key::Right;
//         case SDL_SCANCODE_LEFT:         return (int)Key::Left;
//         case SDL_SCANCODE_DOWN:         return (int)Key::Down;
//         case SDL_SCANCODE_UP:           return (int)Key::Up;
//         case SDL_SCANCODE_PAGEUP:       return (int)Key::PageUp;
//         case SDL_SCANCODE_PAGEDOWN:     return (int)Key::PageDown;
//         case SDL_SCANCODE_HOME:         return (int)Key::Home;
//         case SDL_SCANCODE_END:          return (int)Key::End;
//         case SDL_SCANCODE_CAPSLOCK:     return (int)Key::CapsLock;
//         case SDL_SCANCODE_SCROLLLOCK:   return (int)Key::ScrollLock;
//         case SDL_SCANCODE_NUMLOCKCLEAR: return (int)Key::NumLock;
//         case SDL_SCANCODE_PRINTSCREEN:  return (int)Key::PrintScreen;
//         case SDL_SCANCODE_PAUSE:        return (int)Key::Pause;

//         case SDL_SCANCODE_F1:           return (int)Key::F1;
//         case SDL_SCANCODE_F2:           return (int)Key::F2;
//         case SDL_SCANCODE_F3:           return (int)Key::F3;
//         case SDL_SCANCODE_F4:           return (int)Key::F4;
//         case SDL_SCANCODE_F5:           return (int)Key::F5;
//         case SDL_SCANCODE_F6:           return (int)Key::F6;
//         case SDL_SCANCODE_F7:           return (int)Key::F7;
//         case SDL_SCANCODE_F8:           return (int)Key::F8;
//         case SDL_SCANCODE_F9:           return (int)Key::F9;
//         case SDL_SCANCODE_F10:          return (int)Key::F10;
//         case SDL_SCANCODE_F11:          return (int)Key::F11;
//         case SDL_SCANCODE_F12:          return (int)Key::F12;

//         case SDL_SCANCODE_LSHIFT:       return (int)Key::LeftShift;
//         case SDL_SCANCODE_LCTRL:        return (int)Key::LeftControl;
//         case SDL_SCANCODE_LALT:         return (int)Key::LeftAlt;
//         case SDL_SCANCODE_LGUI:         return (int)Key::LeftSuper;
//         case SDL_SCANCODE_RSHIFT:       return (int)Key::RightShift;
//         case SDL_SCANCODE_RCTRL:        return (int)Key::RightControl;
//         case SDL_SCANCODE_RALT:         return (int)Key::RightAlt;
//         case SDL_SCANCODE_RGUI:         return (int)Key::RightSuper;
//         case SDL_SCANCODE_APPLICATION:  return (int)Key::Menu;
//         default:                        return (int)Key::Unknown;
//     }
// }

static uint32_t ToSDLMouseMask(MouseButton b)
{
    switch (b)
    {
        case MouseButton::Left:   return SDL_BUTTON_MASK(SDL_BUTTON_LEFT);
        case MouseButton::Right:  return SDL_BUTTON_MASK(SDL_BUTTON_RIGHT);
        case MouseButton::Middle: return SDL_BUTTON_MASK(SDL_BUTTON_MIDDLE);
        case MouseButton::X1:     return SDL_BUTTON_MASK(SDL_BUTTON_X1);
        case MouseButton::X2:     return SDL_BUTTON_MASK(SDL_BUTTON_X2);
        default:                  return 0;
    }
}

static SDL_Scancode ToSDLScancode(Key k)
{
    switch (k)
    {
        case Key::Space:          return SDL_SCANCODE_SPACE;
        case Key::Apostrophe:     return SDL_SCANCODE_APOSTROPHE;
        case Key::Comma:          return SDL_SCANCODE_APOSTROPHE;
        case Key::Minus:          return SDL_SCANCODE_MINUS;
        case Key::Period:         return SDL_SCANCODE_PERIOD;
        case Key::Slash:          return SDL_SCANCODE_SLASH;
        case Key::D0:             return SDL_SCANCODE_0;
        case Key::D1:             return SDL_SCANCODE_1;
        case Key::D2:             return SDL_SCANCODE_2;
        case Key::D3:             return SDL_SCANCODE_3;
        case Key::D4:             return SDL_SCANCODE_4;
        case Key::D5:             return SDL_SCANCODE_5;
        case Key::D6:             return SDL_SCANCODE_6;
        case Key::D7:             return SDL_SCANCODE_7;
        case Key::D8:             return SDL_SCANCODE_8;
        case Key::D9:             return SDL_SCANCODE_9;
        case Key::A:              return SDL_SCANCODE_A;
        case Key::B:              return SDL_SCANCODE_B;
        case Key::C:              return SDL_SCANCODE_C;
        case Key::D:              return SDL_SCANCODE_D;
        case Key::E:              return SDL_SCANCODE_E;
        case Key::F:              return SDL_SCANCODE_F;
        case Key::G:              return SDL_SCANCODE_G;
        case Key::H:              return SDL_SCANCODE_H;
        case Key::I:              return SDL_SCANCODE_I;
        case Key::J:              return SDL_SCANCODE_J;
        case Key::K:              return SDL_SCANCODE_K;
        case Key::L:              return SDL_SCANCODE_L;
        case Key::M:              return SDL_SCANCODE_M;
        case Key::N:              return SDL_SCANCODE_N;
        case Key::O:              return SDL_SCANCODE_O;
        case Key::P:              return SDL_SCANCODE_P;
        case Key::Q:              return SDL_SCANCODE_Q;
        case Key::R:              return SDL_SCANCODE_R;
        case Key::S:              return SDL_SCANCODE_S;
        case Key::T:              return SDL_SCANCODE_T;
        case Key::U:              return SDL_SCANCODE_U;
        case Key::V:              return SDL_SCANCODE_V;
        case Key::W:              return SDL_SCANCODE_W;
        case Key::X:              return SDL_SCANCODE_X;
        case Key::Y:              return SDL_SCANCODE_Y;
        case Key::Z:              return SDL_SCANCODE_Z;
        case Key::Escape:         return SDL_SCANCODE_ESCAPE;
        case Key::Enter:          return SDL_SCANCODE_RETURN;
        case Key::Tab:            return SDL_SCANCODE_TAB;
        case Key::Backspace:      return SDL_SCANCODE_BACKSPACE;
        case Key::Insert:         return SDL_SCANCODE_INSERT;
        case Key::Delete:         return SDL_SCANCODE_DELETE;
        case Key::Right:          return SDL_SCANCODE_RIGHT;
        case Key::Left:           return SDL_SCANCODE_LEFT;
        case Key::Down:           return SDL_SCANCODE_DOWN;
        case Key::Up:             return SDL_SCANCODE_UP;
        case Key::PageUp:         return SDL_SCANCODE_PAGEUP;
        case Key::PageDown:       return SDL_SCANCODE_PAGEDOWN;
        case Key::Home:           return SDL_SCANCODE_HOME;
        case Key::End:            return SDL_SCANCODE_END;
        case Key::CapsLock:       return SDL_SCANCODE_CAPSLOCK;
        case Key::ScrollLock:     return SDL_SCANCODE_SCROLLLOCK;
        case Key::NumLock:        return SDL_SCANCODE_NUMLOCKCLEAR;
        case Key::PrintScreen:    return SDL_SCANCODE_PRINTSCREEN;
        case Key::Pause:          return SDL_SCANCODE_PAUSE;
        case Key::F1:             return SDL_SCANCODE_F1;
        case Key::F2:             return SDL_SCANCODE_F2;
        case Key::F3:             return SDL_SCANCODE_F3;
        case Key::F4:             return SDL_SCANCODE_F4;
        case Key::F5:             return SDL_SCANCODE_F5;
        case Key::F6:             return SDL_SCANCODE_F6;
        case Key::F7:             return SDL_SCANCODE_F7;
        case Key::F8:             return SDL_SCANCODE_F8;
        case Key::F9:             return SDL_SCANCODE_F9;
        case Key::F10:            return SDL_SCANCODE_F10;
        case Key::F11:            return SDL_SCANCODE_F11;
        case Key::F12:            return SDL_SCANCODE_F12;
        case Key::LeftShift:      return SDL_SCANCODE_LSHIFT;
        case Key::LeftControl:    return SDL_SCANCODE_LCTRL;
        case Key::LeftAlt:        return SDL_SCANCODE_LALT;
        case Key::LeftSuper:      return SDL_SCANCODE_LGUI;
        case Key::RightShift:     return SDL_SCANCODE_RSHIFT;
        case Key::RightControl:   return SDL_SCANCODE_RCTRL;
        case Key::RightAlt:       return SDL_SCANCODE_RALT;
        case Key::RightSuper:     return SDL_SCANCODE_RGUI;
        case Key::Menu:           return SDL_SCANCODE_APPLICATION;
        default:                  return SDL_SCANCODE_UNKNOWN;
    }
    
}
