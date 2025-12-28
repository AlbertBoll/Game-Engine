#pragma once
#include "Event.h"

class AppQuitEvent final : public Event {
public:
    AppQuitEvent() = default;
    EVENT_CLASS_TYPE(AppQuit)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class AppTickEvent final: public Event
{
public:
    AppTickEvent() = default;
    EVENT_CLASS_TYPE(AppTick)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class AppUpdateEvent final: public Event
{
public:
    AppUpdateEvent() = default;
    EVENT_CLASS_TYPE(AppUpdate)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class AppRenderEvent final: public Event
{
public:
    AppRenderEvent() = default;
    EVENT_CLASS_TYPE(AppRender)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
};