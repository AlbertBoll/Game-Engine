#pragma once

#include<vector>
#include"Layer.h"
#include"Base.h"

class LayerStack
{
    using Iterator = std::vector<Layer*>::iterator;
    using ReverseIterator = std::vector<Layer*>::reverse_iterator;
    using ConstIterator = std::vector<Layer*>::const_iterator;
    using ConstReverseIterator = std::vector<Layer*>::const_reverse_iterator;
public:
    LayerStack() = default;
    ~LayerStack();

    void PushLayer(Layer* layer);
    void PushOverlay(Layer* overlay);
    void PopLayer(Layer* layer);
    void PopOverlay(Layer* overlay);

    Iterator begin() { return m_Layers.begin(); }
    Iterator end() { return m_Layers.end(); }
    ReverseIterator rbegin() { return m_Layers.rbegin(); }
    ReverseIterator rend() { return m_Layers.rend(); }

    ConstIterator begin() const { return m_Layers.begin(); }
    ConstIterator end()	const { return m_Layers.end(); }
    ConstReverseIterator rbegin() const { return m_Layers.rbegin(); }
    ConstReverseIterator rend() const { return m_Layers.rend(); }
private:
    std::vector<Layer*> m_Layers;
    size_t m_LayerInsertIndex = 0;
};