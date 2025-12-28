#pragma once

#include"Event.h"

class MouseMovedEvent final : public Event {
public:
    MouseMovedEvent(float x, float y, float dx = 0.0f, float dy = 0.0f)
        : m_X(x), m_Y(y), m_dX(dx), m_dY(dy) {}
    float GetX()  const { return m_X; }
    float GetY()  const { return m_Y; }
    float GetDX() const { return m_dX; }
    float GetDY() const { return m_dY; }

    std::string ToString() const override {
        return std::string(GetName()) + " x=" + std::to_string(m_X) +
               " y=" + std::to_string(m_Y) + " dx=" + std::to_string(m_dX) +
               " dy=" + std::to_string(m_dY);
    }

    EVENT_CLASS_TYPE(MouseMoved)
    EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse)

private:
    float m_X, m_Y;
    float m_dX, m_dY; // relative move
};

class MouseScrolledEvent final : public Event {
public:
    MouseScrolledEvent(float offsetX, float offsetY)
        : m_OffsetX(offsetX), m_OffsetY(offsetY) {}
    float GetXOffset() const { return m_OffsetX; }
    float GetYOffset() const { return m_OffsetY; }

    std::string ToString() const override {
        return std::string(GetName()) + " (" + std::to_string(m_OffsetX) +
               ", " + std::to_string(m_OffsetY) + ")";
    }

    EVENT_CLASS_TYPE(MouseScrolled)
    EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse)

private:
    float m_OffsetX, m_OffsetY;
};

class MouseButtonEvent : public Event
{
public:
    MouseCode GetMouseButton() const { return m_Button; }

    EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton)
protected:
    MouseButtonEvent(const MouseCode button)
        : m_Button(button) {}

    MouseCode m_Button;
};

class MouseButtonPressedEvent : public MouseButtonEvent
{
public:
    MouseButtonPressedEvent(const MouseCode button)
        : MouseButtonEvent(button) {}

    std::string ToString() const override
    {
        std::stringstream ss;
        ss << "MouseButtonPressedEvent: " << m_Button;
        return ss.str();
    }

    EVENT_CLASS_TYPE(MouseButtonPressed)
};

class MouseButtonReleasedEvent : public MouseButtonEvent
{
public:
    MouseButtonReleasedEvent(const MouseCode button)
        : MouseButtonEvent(button) {}

    std::string ToString() const override
    {
        std::stringstream ss;
        ss << "MouseButtonReleasedEvent: " << m_Button;
        return ss.str();
    }

    EVENT_CLASS_TYPE(MouseButtonReleased)
};
