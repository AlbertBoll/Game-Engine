#pragma once
#include <cstdint>
#include "Core/Base.h"

using Handle = std::uint32_t;

class VertexBuffer;
class IndexBuffer;

class VertexArray
{
public:
    VertexArray();
    ~VertexArray();

    MOVEONLY(VertexArray)

    void Bind() const;
    void Unbind() const;

    void AddVertexBuffer(const VertexBuffer& vertexBuffer);
    void SetIndexBuffer(const IndexBuffer& indexBuffer);

    uint32_t GetRendererID() const { return m_RendererID; }
  
private:
    Handle m_RendererID = 0;
    uint32_t m_NextAttrib = 0;
};