#include"Renderer/IndexBuffer.h"
#include <glad/gl.h>

static GLenum ToGLIndexType(IndexType t)
{
    return (t == IndexType::U16) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
}

std::size_t IndexBuffer::GetIndexStrideBytes() const
{
    return (m_Type == IndexType::U16) ? sizeof(uint16_t) : sizeof(uint32_t);
}

IndexBuffer::IndexBuffer(std::size_t indexCount, IndexType type, BufferUsage usage, const void* initialData)
    : m_Count(indexCount), m_Type(type), m_Usage(usage)
{
    glGenBuffers(1, &m_RendererID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);

    const std::size_t bytes = m_Count * GetIndexStrideBytes();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)bytes, initialData, ToGLUsage(m_Usage));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

IndexBuffer::~IndexBuffer()
{
    if (m_RendererID != 0)
        glDeleteBuffers(1, &m_RendererID);
}

IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept
{
    *this = std::move(other);
}

IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept
{
    if (this == &other) return *this;

    if (m_RendererID) glDeleteBuffers(1, &m_RendererID);

    m_RendererID = other.m_RendererID;
    m_Count = other.m_Count;
    m_Type = other.m_Type;
    m_Usage = other.m_Usage;

    other.m_RendererID = 0;
    other.m_Count = 0;
    return *this;
}

void IndexBuffer::Bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
}

void IndexBuffer::Unbind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

bool IndexBuffer::Resize(std::size_t newIndexCount)
{
    if (!m_RendererID) return false;

    m_Count = newIndexCount;
    const std::size_t bytes = m_Count * GetIndexStrideBytes();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)bytes, nullptr, ToGLUsage(m_Usage));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    return true;
}

bool IndexBuffer::SetData(const void* indices, std::size_t indexCount, BufferUpdateMode mode)
{
    if (!m_RendererID || !indices || indexCount == 0) return false;

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);

    const std::size_t newBytes = indexCount * GetIndexStrideBytes();
    const std::size_t oldBytes = m_Count * GetIndexStrideBytes();

    if (indexCount != m_Count)
    {
        m_Count = indexCount;
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)newBytes, indices, ToGLUsage(m_Usage));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        return true;
    }

    if (mode == BufferUpdateMode::OrphanThenSubData)
    {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)oldBytes, nullptr, ToGLUsage(m_Usage));
    }

    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, (GLsizeiptr)oldBytes, indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    return true;
}

bool IndexBuffer::UpdateSubData(std::size_t indexOffset, const void* indices, std::size_t indexCount)
{
    if (!m_RendererID || !indices || indexCount == 0) return false;
    if (indexOffset + indexCount > m_Count) return false;

    const std::size_t stride = GetIndexStrideBytes();
    const std::size_t offsetBytes = indexOffset * stride;
    const std::size_t sizeBytes = indexCount * stride;

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, (GLintptr)offsetBytes, (GLsizeiptr)sizeBytes, indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    return true;
}

uint32_t IndexBuffer::GetCount() const
{
    return m_Count;
}