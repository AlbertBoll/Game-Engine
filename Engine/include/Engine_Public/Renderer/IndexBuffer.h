#pragma once
#include <cstdint>
#include "Buffer.h"
#include "Core/Base.h"

enum class IndexType : uint8_t
{
    U16,
    U32
};

class IndexBuffer
{
public:
    IndexBuffer(std::size_t indexCount,
                IndexType type = IndexType::U32,
                BufferUsage usage = BufferUsage::StaticDraw,
                const void* initialData = nullptr);

    ~IndexBuffer();

    MOVEONLY(IndexBuffer)

    void Bind() const;
    void Unbind() const;

    uint32_t GetCount() const;
    uint32_t GetRendererID() const;

    IndexType    GetIndexType() const { return m_Type; }
    std::size_t  GetIndexStrideBytes() const;

    bool Resize(std::size_t newIndexCount);
    
    bool SetData(const void* indices,
                std::size_t indexCount,
                BufferUpdateMode mode = BufferUpdateMode::SubData);

    bool UpdateSubData(std::size_t indexOffset,
                       const void* indices,
                       std::size_t indexCount);

private:
    Handle m_RendererID = 0;
    uint32_t m_Count = 0;
    IndexType m_Type = IndexType::U32;
    BufferUsage m_Usage = BufferUsage::StaticDraw;
};