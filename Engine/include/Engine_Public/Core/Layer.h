#pragma once

#include "Base.h"
#include "Events/Event.h"
#include "Timestep.h"

class ENGINE_API Layer
{
public:
    Layer(const std::string& name = "Layer");
    virtual ~Layer() = default;

    virtual void OnAttach() {}
    virtual void OnDetach() {}
    virtual void OnUpdate(Timestep ts) {}
    virtual void OnEvent(Event& event) {}
    bool GetPendingDestroy() const {return b_PendingDestroy;}

    const std::string& GetName() const { return m_LayerName; }
protected:
    std::string m_LayerName;
    bool b_PendingDestroy{false};
};