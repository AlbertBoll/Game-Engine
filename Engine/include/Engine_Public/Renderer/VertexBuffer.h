#pragma once

#include"BufferLayout.h"
#include"Buffer.h"
#include <cstdint>


class VertexBuffer
{
public:
    VertexBuffer(std::size_t sizeBytes,
                 BufferUsage usage = BufferUsage::StaticDraw,
                 const void* initialData = nullptr);

    VertexBuffer(const void* data, uint32_t sizeBytes);
   
    ~VertexBuffer();
   
    MOVEONLY(VertexBuffer)


    void Bind() const;
    void Unbind() const;

    void SetLayout(const BufferLayout& layout);
    const BufferLayout& GetLayout() const;
    std::size_t  GetSizeBytes() const { return m_SizeBytes; }
    uint32_t GetRendererID() const;
    BufferUsage  GetUsage() const { return m_Usage; }


    bool Resize(std::size_t newSizeBytes);


    bool SetData(const void* data,
                 std::size_t sizeBytes,
                 BufferUpdateMode mode = BufferUpdateMode::SubData);

    bool UpdateSubData(std::size_t offsetBytes,
                       const void* data,
                       std::size_t sizeBytes);


private:
    Handle m_RendererID = 0;
    std::size_t m_SizeBytes = 0;
    BufferUsage m_Usage = BufferUsage::StaticDraw;
    BufferLayout m_Layout;
};

