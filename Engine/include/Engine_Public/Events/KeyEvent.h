#pragma once
#include "Event.h"

class KeyPressedEvent final : public Event {
public:
    KeyPressedEvent(int keycode, int repeatCount)
        : m_KeyCode(keycode), m_RepeatCount(repeatCount) {}

    int GetKeyCode() const { return m_KeyCode; }
    int GetRepeatCount() const { return m_RepeatCount; }

    std::string ToString() const override {
        return std::string(GetName()) + " key=" + std::to_string(m_KeyCode) +
               " repeat=" + std::to_string(m_RepeatCount);
    }

    EVENT_CLASS_TYPE(KeyPressed)
    EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryKeyboard)

private:
    int m_KeyCode;
    int m_RepeatCount; // 0=non repeat，>0 represent repeat
};


class KeyReleasedEvent final : public Event {
public:
    explicit KeyReleasedEvent(int keycode) : m_KeyCode(keycode) {}
    int GetKeyCode() const { return m_KeyCode; }

    std::string ToString() const override {
        return std::string(GetName()) + " key=" + std::to_string(m_KeyCode);
    }

    EVENT_CLASS_TYPE(KeyReleased)
    EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryKeyboard)

private:
    int m_KeyCode;
};

class KeyTypedEvent final : public Event {
public:
    explicit KeyTypedEvent(uint32_t codepoint) : m_Codepoint(codepoint) {}
    uint32_t GetCodepoint() const { return m_Codepoint; }

    EVENT_CLASS_TYPE(KeyTyped)
    EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryKeyboard)

private:
    uint32_t m_Codepoint; // Unicode code point
};