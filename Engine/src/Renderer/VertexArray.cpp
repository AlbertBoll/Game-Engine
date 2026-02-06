#include <glad/gl.h>
#include"Renderer/VertexArray.h"
#include"Renderer/VertexBuffer.h"
#include"Renderer/IndexBuffer.h"


static constexpr GLenum ToGLBaseType(ShaderDataType t)
{
    switch (t)
    {
    case ShaderDataType::Float:  
    case ShaderDataType::Float2:
    case ShaderDataType::Float3:
    case ShaderDataType::Float4: 
    case ShaderDataType::Mat3:
    case ShaderDataType::Mat4:
        return GL_FLOAT;

    case ShaderDataType::Double:
    case ShaderDataType::Double2:
    case ShaderDataType::Double3:
    case ShaderDataType::Double4:
        return GL_DOUBLE;

    case ShaderDataType::Int:  
    case ShaderDataType::Int2: 
    case ShaderDataType::Int3: 
    case ShaderDataType::Int4: 
        return GL_INT;

    case ShaderDataType::UInt:  
    case ShaderDataType::UInt2: 
    case ShaderDataType::UInt3: 
    case ShaderDataType::UInt4: 
        return GL_UNSIGNED_INT;
        
    case ShaderDataType::Short:  
    case ShaderDataType::Short2: 
    case ShaderDataType::Short3: 
    case ShaderDataType::Short4: 
        return GL_SHORT;

    case ShaderDataType::UShort:  
    case ShaderDataType::UShort2: 
    case ShaderDataType::UShort3: 
    case ShaderDataType::UShort4: 
        return GL_UNSIGNED_SHORT;

    case ShaderDataType::Byte:  
    case ShaderDataType::Byte2: 
    case ShaderDataType::Byte3: 
    case ShaderDataType::Byte4: 
        return GL_BYTE;

    case ShaderDataType::UByte:  
    case ShaderDataType::UByte2: 
    case ShaderDataType::UByte3: 
    case ShaderDataType::UByte4: 
    case ShaderDataType::Bool:
        return GL_UNSIGNED_BYTE;

    default:
        return GL_FLOAT;
    }
}

VertexArray::VertexArray()
{
    glGenVertexArrays(1, &m_RendererID);
}

VertexArray::~VertexArray()
{
    if (m_RendererID != 0)
        glDeleteVertexArrays(1, &m_RendererID);
}

void VertexArray::Bind() const
{
    glBindVertexArray(m_RendererID);
}

void VertexArray::Unbind() const
{
    glBindVertexArray(0);
}

void VertexArray::AddVertexBuffer(const VertexBuffer& vertexBuffer)
{
    Bind();
    vertexBuffer.Bind();
    const BufferLayout& layout = vertexBuffer.GetLayout();
    for (const auto& element : layout)
    {
        const GLenum glBaseType = ToGLBaseType(element.m_Type);

        if(IsMatrixAttrib(element.m_Type))
        {
            const uint32_t cols = element.GetAttribSlotCount();
            const uint32_t rows = element.GetSlotComponentCount();
            const uint32_t colStrideBytes = BaseTypeSize(element.m_Type) * rows;
            for (uint32_t slot = 0; slot < cols; ++slot)
            {
                glEnableVertexAttribArray(m_NextAttrib);
                glVertexAttribPointer(m_NextAttrib,
                                      rows,
                                      glBaseType,
                                      element.m_Normalized ? GL_TRUE : GL_FALSE,
                                      layout.GetStride(),
                                      OffsetPtr(element.m_Offset + colStrideBytes * slot));
                glVertexAttribDivisor(m_NextAttrib, element.m_Divisor);
                m_NextAttrib++;
            }
        }
        else
        {
            glEnableVertexAttribArray(m_NextAttrib);
            if(glBaseType == GL_DOUBLE)
            {
                glVertexAttribLPointer(
                    m_NextAttrib,
                    element.GetComponentCount(),
                    glBaseType,
                    layout.GetStride(),
                    OffsetPtr(element.m_Offset));
            }
            else if(IsIntegralAttrib(element.m_Type) && !element.m_Normalized)
            {
                glVertexAttribIPointer(m_NextAttrib,
                                       element.GetComponentCount(),
                                       glBaseType,
                                       layout.GetStride(),
                                       OffsetPtr(element.m_Offset));
            }
            else
            {
                glVertexAttribPointer(
                    m_NextAttrib,
                    element.GetComponentCount(),
                    glBaseType,
                    element.m_Normalized ? GL_TRUE : GL_FALSE,
                    layout.GetStride(),
                    OffsetPtr(element.m_Offset));
            }

            glVertexAttribDivisor(m_NextAttrib, element.m_Divisor);
            m_NextAttrib++;
        }
 
    }

    vertexBuffer.Unbind();
    Unbind();
}

void VertexArray::SetIndexBuffer(const IndexBuffer& indexBuffer)
{
    Bind();
    indexBuffer.Bind();
    Unbind();
}