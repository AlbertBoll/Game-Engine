#pragma once
#include "Core/Base.h"
#include <vector>

static inline std::size_t AlignUpPow2(std::size_t value, std::size_t alignment)
{
    CORE_ASSERT(alignment != 0, "Alignment must be greater than zero!");
    CORE_ASSERT((alignment & (alignment - 1)) == 0, "Alignment must be a power of two!");
    return (value + alignment - 1) & ~(alignment - 1);
}

// ---------- concept: cmd payload requirement ----------
template<class Cmd, class... Args>
concept CommandPayload =
    std::is_trivially_destructible_v<Cmd> &&
    std::constructible_from<Cmd, Args...> &&
    requires(const void* p) {
        { Cmd::Execute(p) } -> std::same_as<void>;
    };


class CommandQueue
{
public:
    explicit CommandQueue(std::size_t initialReserveBytes = 1 << 20) // 1MB
    {
        ReserveAtLeast(initialReserveBytes);
    }

    void Reset()
    {
        // keep memory, just reset write head
        m_UsedBytes = 0;
    }

    bool Empty() const { return m_UsedBytes == 0; }
    std::size_t UsedBytes() const { return m_UsedBytes; }
    std::size_t CapacityBytes() const { return m_Buffer.capacity(); }

    template<class Cmd, class... Args>
    requires CommandPayload<Cmd, Args...>
    void Enqueue(Args&&... args)
    {
        const std::size_t headerAlign  = alignof(CommandHeader);
        const std::size_t payloadAlign = alignof(Cmd);

        // compute where header/payload will land inside the buffer
        std::size_t headerPos  = AlignUpPow2(m_UsedBytes, headerAlign);
        std::size_t payloadPos = AlignUpPow2(headerPos + sizeof(CommandHeader), payloadAlign);

        // Align end so next header is always aligned (executor gets simpler)
        std::size_t endPos = AlignUpPow2(payloadPos + sizeof(Cmd), headerAlign);
      
        EnsureWritable(endPos); // may reallocate; do this BEFORE taking pointers

        // write header
        //auto* header = reinterpret_cast<CommandHeader*>(m_Buffer.data() + headerPos);
        // placement-new header to start lifetime cleanly
        void* headerMem = m_Buffer.data() + headerPos;
        auto* header = new (headerMem) CommandHeader{};
        header->m_Execute = &Cmd::Execute;
        header->m_SizeBytes = static_cast<uint32_t>(endPos - headerPos);
        header->m_PayloadOffsetBytes = static_cast<uint32_t>(payloadPos - headerPos);

        // construct payload in-place
        void* payloadPtr = m_Buffer.data() + payloadPos;
        new (payloadPtr) Cmd{ std::forward<Args>(args)... };

        // advance write head
        m_UsedBytes = endPos;
    }

    void ExecuteAll()
    {
        std::size_t cursor = 0;
        //const std::size_t headerAlign = alignof(CommandHeader);

        while (cursor < m_UsedBytes)
        {
            //cursor = AlignUpPow2(cursor, headerAlign);

            auto* header = reinterpret_cast<const CommandHeader*>(m_Buffer.data() + cursor);
            CORE_ASSERT(header->m_SizeBytes > 0, "CommandHeader sizeBytes must be > 0");

            const void* payload = m_Buffer.data() + cursor + header->m_PayloadOffsetBytes;
            header->m_Execute(payload);

            cursor += header->m_SizeBytes;
        }
    }

private:
    using ExecuteFn = void(*)(const void*);

    struct CommandHeader
    {
        ExecuteFn m_Execute = nullptr;
        uint32_t m_SizeBytes = 0;
        uint32_t m_PayloadOffsetBytes = 0; // use 32-bit to avoid edge overflow
    };

private:
    // Reserve to at least N bytes capacity, and make size == capacity once (so all bytes are writable)
    void ReserveAtLeast(std::size_t bytes)
    {
        if (bytes <= m_Buffer.capacity())
            return;

        m_Buffer.reserve(bytes);

        // Important: make bytes writable up to capacity.
        // This "resize to capacity" only happens when capacity grows,
        // so don't pay this cost on every Enqueue.
        m_Buffer.resize(m_Buffer.capacity()); // make [0, size) writable
    }

    static std::size_t NextCapacity(std::size_t currentCap, std::size_t needed)
    {
        // grow by doubling (common strategy)
        std::size_t cap = currentCap ? currentCap : 256;
        while (cap < needed) cap *= 2;
        return cap;
    }

    void EnsureWritable(std::size_t neededBytes)
    {
        // Ensure our vector size is at least neededBytes so writes are within [0, size).
        // We keep size == capacity (after ReserveAtLeast), so normally this just checks capacity.
        if (neededBytes <= m_Buffer.size())
            return;

        // Need more capacity; reserve bigger and then resize to capacity (writable range)
        const std::size_t newCap = NextCapacity(m_Buffer.capacity(), neededBytes);
        ReserveAtLeast(newCap);
        CORE_ASSERT(m_Buffer.size() >= neededBytes, "EnsureWritable failed");
    }

private:
    std::vector<std::byte> m_Buffer;
    std::size_t m_UsedBytes = 0;
};