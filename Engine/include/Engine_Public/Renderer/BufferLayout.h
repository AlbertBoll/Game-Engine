#pragma once

#include <cstdint>
#include <utility>
#include <vector>
#include "Core/Base.h"

enum class ShaderDataType {
    None = 0,
    Float, Float2, Float3, Float4,
    Double, Double2, Double3, Double4,
    Int, Int2, Int3, Int4,
    UInt, UInt2, UInt3, UInt4,
    Short, Short2, Short3, Short4,
    UShort, UShort2, UShort3, UShort4,
    Byte, Byte2, Byte3, Byte4,
    UByte, UByte2, UByte3, UByte4,
    Mat3, Mat4, Bool
};

static constexpr bool IsMatrixAttrib(ShaderDataType t)
{
    return t == ShaderDataType::Mat3 || t == ShaderDataType::Mat4;
}

static constexpr bool IsIntegralAttrib(ShaderDataType t)
{
    switch (t)
    {
    case ShaderDataType::Int:
    case ShaderDataType::Int2:
    case ShaderDataType::Int3:
    case ShaderDataType::Int4:
    case ShaderDataType::UInt:
    case ShaderDataType::UInt2:
    case ShaderDataType::UInt3:
    case ShaderDataType::UInt4:
    case ShaderDataType::Short:
    case ShaderDataType::Short2:
    case ShaderDataType::Short3:
    case ShaderDataType::Short4:
    case ShaderDataType::UShort:
    case ShaderDataType::UShort2:
    case ShaderDataType::UShort3:
    case ShaderDataType::UShort4:
    case ShaderDataType::Byte:
    case ShaderDataType::Byte2:
    case ShaderDataType::Byte3:
    case ShaderDataType::Byte4:
    case ShaderDataType::UByte:
    case ShaderDataType::UByte2:
    case ShaderDataType::UByte3:
    case ShaderDataType::UByte4:
    case ShaderDataType::Bool:
        return true;
    default:
        return false;
    }
}

static constexpr uint32_t GetAttribSlotCount(ShaderDataType t)
{
    switch (t)
    {
        case ShaderDataType::Mat3: return 3; // three columns => 3 attribs
        case ShaderDataType::Mat4: return 4; // four columns => 4 attribs
        default: return 1;
    }
}

static constexpr uint32_t BaseTypeSize(ShaderDataType t)
{
    switch (t)
    {
    case ShaderDataType::Float:
    case ShaderDataType::Float2:
    case ShaderDataType::Float3:
    case ShaderDataType::Float4:
    case ShaderDataType::Mat3:
    case ShaderDataType::Mat4:
        return 4;

    case ShaderDataType::Double:
    case ShaderDataType::Double2:
    case ShaderDataType::Double3:
    case ShaderDataType::Double4:
        return 8;

    case ShaderDataType::Int:
    case ShaderDataType::Int2:
    case ShaderDataType::Int3:
    case ShaderDataType::Int4:
    case ShaderDataType::UInt:
    case ShaderDataType::UInt2:
    case ShaderDataType::UInt3:
    case ShaderDataType::UInt4:
        return 4;

    case ShaderDataType::Short:
    case ShaderDataType::Short2:
    case ShaderDataType::Short3:
    case ShaderDataType::Short4:
    case ShaderDataType::UShort:
    case ShaderDataType::UShort2:
    case ShaderDataType::UShort3:
    case ShaderDataType::UShort4:
        return 2;

    case ShaderDataType::Byte:
    case ShaderDataType::Byte2:
    case ShaderDataType::Byte3:
    case ShaderDataType::Byte4:
    case ShaderDataType::UByte:
    case ShaderDataType::UByte2:
    case ShaderDataType::UByte3:
    case ShaderDataType::UByte4:
    case ShaderDataType::Bool:
        return 1;

    default: return 0;
    }
}

static constexpr uint32_t GetComponentCount(ShaderDataType t) {
    switch (t) {
        case ShaderDataType::Float:  return 1;
        case ShaderDataType::Float2: return 2;
        case ShaderDataType::Float3: return 3;
        case ShaderDataType::Float4: return 4;

        case ShaderDataType::Double:  return 1;
        case ShaderDataType::Double2: return 2;
        case ShaderDataType::Double3: return 3;
        case ShaderDataType::Double4: return 4;

        case ShaderDataType::Int:  return 1;
        case ShaderDataType::Int2: return 2;
        case ShaderDataType::Int3: return 3;
        case ShaderDataType::Int4: return 4;

        case ShaderDataType::UInt:  return 1;
        case ShaderDataType::UInt2: return 2;
        case ShaderDataType::UInt3: return 3;
        case ShaderDataType::UInt4: return 4;

        case ShaderDataType::Short:  return 1;
        case ShaderDataType::Short2: return 2;
        case ShaderDataType::Short3: return 3;
        case ShaderDataType::Short4: return 4;

        case ShaderDataType::UShort:  return 1;
        case ShaderDataType::UShort2: return 2;
        case ShaderDataType::UShort3: return 3;
        case ShaderDataType::UShort4: return 4;

        case ShaderDataType::Byte:  return 1;
        case ShaderDataType::Byte2: return 2;
        case ShaderDataType::Byte3: return 3;
        case ShaderDataType::Byte4: return 4;

        case ShaderDataType::UByte:  return 1;
        case ShaderDataType::UByte2: return 2;
        case ShaderDataType::UByte3: return 3;
        case ShaderDataType::UByte4: return 4;

        case ShaderDataType::Bool: return 1;
        default: return 0;
    }
}


static constexpr uint32_t ShaderDataTypeSize(ShaderDataType t)
{
    if (IsMatrixAttrib(t))
    {
        const uint32_t n = (t == ShaderDataType::Mat3) ? 3u : 4u;
        return BaseTypeSize(t) * n * n;
    }
    return BaseTypeSize(t) * GetComponentCount(t);
}

static constexpr uint32_t GetTotalComponentCount(ShaderDataType t)
{
    switch (t)
    {
    case ShaderDataType::Mat3: return 9;
    case ShaderDataType::Mat4: return 16;
    default: return GetComponentCount(t);
    }
}

static constexpr uint32_t GetSlotComponentCount(ShaderDataType t)
{
    switch (t)
    {
        case ShaderDataType::Mat3: return 3; // each column vec3
        case ShaderDataType::Mat4: return 4; // each column vec4
        default: return GetComponentCount(t); // non-matrix is the component count
    }
}


struct BufferElement {
    ShaderDataType m_Type = ShaderDataType::None;
    std::string m_Name;
    bool m_Normalized{ false };
    uint32_t m_Divisor{ 0 }; // for instanced rendering: 0=per-vertex, 1=per-instance
    uint32_t m_Offset{ 0 };

    BufferElement() = default;

    // explicit BufferElement(ShaderDataType type, bool normalized, uint32_t divisor = 0)
    //     : m_Type(type), m_Name(), m_Offset(0),
    //       m_Normalized(normalized), m_Divisor(divisor) {}

    BufferElement(ShaderDataType type, std::string name, uint32_t offset = 0, bool normalized = false, uint32_t divisor = 0)
        : m_Type(type), m_Name(std::move(name)), m_Normalized(normalized), m_Divisor(divisor), m_Offset(offset) {}

    uint32_t GetComponentCount() const {
        CORE_ASSERT(!IsMatrixAttrib(m_Type), "Matrix type: use GetSlotComponentCount()/GetAttribSlotCount()");
        return ::GetComponentCount(m_Type);
    }

    uint32_t GetAttribSlotCount() const { return ::GetAttribSlotCount(m_Type); }

    uint32_t GetSlotComponentCount() const { return ::GetSlotComponentCount(m_Type); }

    uint32_t GetTotalComponentCount() const { return ::GetTotalComponentCount(m_Type); }

    uint32_t GetSize() const {
        return ShaderDataTypeSize(m_Type);
    }
};

class BufferLayout 
{
public:
    using value_type = BufferElement;
    using container_type = std::vector<value_type>;
    using iterator = container_type::iterator;
    using const_iterator = container_type::const_iterator;

    BufferLayout() = default;  

    explicit BufferLayout(container_type elements)
        : m_Elements(std::move(elements)), m_Stride(0) 
    {
        CalculateOffsetsAndStride();
    }

    BufferLayout(container_type elements, uint32_t explicitStride)
        : m_Elements(std::move(elements)), m_Stride(explicitStride)
    {
        
    }


    // BufferLayout(const std::vector<BufferElement>& elements): m_Elements(elements), m_Stride(0) 
    // {
    //    CalculateOffsetsAndStride();
    // }

    inline const container_type& GetElements() const { return m_Elements; }
    inline uint32_t GetStride() const { return m_Stride; }
    iterator begin() { return m_Elements.begin(); }
    iterator end() { return m_Elements.end(); }
    const_iterator begin() const { return m_Elements.begin(); }
    const_iterator end() const { return m_Elements.end(); }

private:
    void CalculateOffsetsAndStride()
    {
        uint32_t offset = 0;
        m_Stride = 0;
        for (auto& element : m_Elements)
        {
            element.m_Offset = offset;
            offset += element.GetSize();
        }
        m_Stride = offset;
    }   


private:
    container_type m_Elements;
    uint32_t m_Stride{ 0 };
};

class BufferLayoutBuilder
{
public:

    BufferLayoutBuilder& AddElement(ShaderDataType type, std::string name, uint32_t offset = 0, bool normalized = false, uint32_t divisor = 0)
    {
        m_Elements.emplace_back(type, std::move(name), offset, normalized, divisor);
        return *this;
    }

    BufferLayoutBuilder& SetStride(uint32_t strideBytes)
    {
        m_ExplicitStride = strideBytes;
        return *this;
    }

    BufferLayout Build()&&
    {
        CORE_ASSERT(!m_Elements.empty(), "BufferLayout must have at least one element!");
        if(m_ExplicitStride!=0)
        {
            return BufferLayout(std::move(m_Elements), m_ExplicitStride);
        }
        return BufferLayout(std::move(m_Elements));
    }
    BufferLayout build()& = delete;

private:
    std::vector<BufferElement> m_Elements;
    uint32_t m_ExplicitStride = 0;
};
