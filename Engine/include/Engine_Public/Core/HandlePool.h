#pragma once
#include <vector>
#include <cstdint>

using u32 = uint32_t;
using u64 = uint64_t;

template<class Slot>
class HandlePool
{
public:
    struct Handle
    {
        u32 id = 0;          // index + 1
        u32 generation = 0;  // version
        explicit operator bool() const { return id != 0; }
        friend bool operator==(const Handle&, const Handle&) = default;
    };

public:
    void Reserve(u32 count) { m_Slots.reserve(count); m_Free.reserve(count); }

    Handle Allocate()
    {
        u32 idx = 0;
        if (!m_Free.empty())
        {
            idx = m_Free.back();
            m_Free.pop_back();
        }
        else
        {
            m_Slots.emplace_back();         // Slot 必须可默认构造
            idx = (u32)m_Slots.size() - 1;
        }

        Slot& s = m_Slots[idx];
        s.alive = true;
        return Handle{ idx + 1, s.generation };
    }

    void Free(Handle h)
    {
        Slot* s = Get(h);
        if (!s) return;

        const u32 idx = h.id - 1;
        s->alive = false;
        s->generation++;
        m_Free.push_back(idx);
    }

    Slot* Get(Handle h)
    {
        if (!h) return nullptr;

        const u32 idx = h.id - 1;
        if (idx >= m_Slots.size()) return nullptr;

        Slot& s = m_Slots[idx];
        if (!s.alive) return nullptr;
        if (s.generation != h.generation) return nullptr;
        return &s;
    }

    const Slot* Get(Handle h) const { return const_cast<HandlePool*>(this)->Get(h); }

    std::vector<Slot>& Slots() { return m_Slots; }
    const std::vector<Slot>& Slots() const { return m_Slots; }

    // 清空“逻辑内容”，但保留 slots/free 的 capacity，且让旧 handle 全部失效
    void ResetKeepCapacity()
    {
        m_Free.clear();
        m_Free.reserve(m_Slots.size());

        for (u32 i = 0; i < (u32)m_Slots.size(); ++i)
        {
            Slot& s = m_Slots[i];
            s.alive = false;
            s.generation++;          // invalidate old handles
            m_Free.push_back(i);
        }
    }

private:
    std::vector<Slot> m_Slots;
    std::vector<u32>  m_Free;
};