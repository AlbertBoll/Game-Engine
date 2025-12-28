#pragma once 

#include <functional>
#include "Core/Base.h"

enum class EventType
{
    None = 0,
    WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved, WindowMinimized, WindowMaximized, WindowRestored,
    AppTick, AppUpdate, AppRender, AppQuit,
    KeyPressed, KeyReleased, KeyTyped,
    MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
};

enum EventCategory
{
    None = 0,
    EventCategoryApplication    = BIT(0),
    EventCategoryInput          = BIT(1),
    EventCategoryKeyboard       = BIT(2),
    EventCategoryMouse          = BIT(3),
    EventCategoryMouseButton    = BIT(4)
};

#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::type; }\
								virtual EventType GetEventType() const override { return GetStaticType(); }\
								virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

class Event
{
public:
    virtual ~Event() = default;

    bool Handled = false;

    virtual EventType GetEventType() const = 0;
    virtual const char* GetName() const = 0;
    virtual int GetCategoryFlags() const = 0;
    virtual std::string ToString() const { return GetName(); }

    bool IsInCategory(EventCategory category)
    {
        return GetCategoryFlags() & category;
    }
};


// 1) Concept: T must be derived from Event
template<typename T>
concept Event_Type = std::derived_from<T, Event> && !std::same_as<T, Event>;

template<typename F, typename T>
concept EventHandler = Event_Type<T> && requires(F f, T& e){
    {std::invoke(f, e)} -> std::same_as<bool>;
};

class EventDispatcher
{
public:
    EventDispatcher(Event& event)
        : m_Event(event)
    {
    }
    
    // F will be deduced by the compiler
    template<Event_Type T, typename F>
    requires EventHandler<F, T>
    bool Dispatch(F&& func)
    {
        if (m_Event.GetEventType() == T::GetStaticType())
        {
            //m_Event.Handled |= func(static_cast<T&>(m_Event));
            m_Event.Handled |= std::invoke(std::forward<F>(func), static_cast<T&>(m_Event));
            return true;
        }
        return false;
    }
private:
    Event& m_Event;
};

inline std::ostream& operator<<(std::ostream& os, const Event& e)
{
    return os << e.ToString();
}
