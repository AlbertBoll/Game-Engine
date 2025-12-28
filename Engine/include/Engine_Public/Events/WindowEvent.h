#pragma once
#include "Event.h"

class WindowCloseEvent final : public Event {
public:
    WindowCloseEvent() = default;
    EVENT_CLASS_TYPE(WindowClose)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class WindowResizeEvent final : public Event {
public:
    WindowResizeEvent(int w, int h) : m_Width(w), m_Height(h) {}
    int GetWidth()  const { return m_Width; }
    int GetHeight() const { return m_Height; }

    std::string ToString() const override {
        return std::string(GetName()) + ": " + std::to_string(m_Width) + "x" + std::to_string(m_Height);
    }

    EVENT_CLASS_TYPE(WindowResize)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)

private:
    int m_Width, m_Height;
};


class WindowMinimizedEvent final : public Event {
public:
    EVENT_CLASS_TYPE(WindowMinimized)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class WindowMaximizedEvent final : public Event {
public:
    EVENT_CLASS_TYPE(WindowMaximized)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class WindowRestoredEvent final : public Event {
public:
    EVENT_CLASS_TYPE(WindowRestored)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
};