#pragma once
#include "Renderer/IndexBuffer.h"
#include "Renderer/VertexBuffer.h"
#include "Renderer/VertexArray.h"


struct PrimVertex
{
    float px,py,pz;      // position
    float u,v;           // texture coordinate
    float nx,ny,nz;      // normal   
    float tx,ty,tz,tw;   // tangent
};


struct MeshGL
{
    VertexArray m_VertexArray;
    VertexBuffer m_VertexBuffer;
    IndexBuffer  m_IndexBuffer;
    uint32_t m_IndexCount = 0;

    MOVE_DEFAULT_ONLY(MeshGL)

    MeshGL(std::vector<PrimVertex>&& verts, std::vector<uint32_t>&& inds)
        : m_VertexBuffer(verts.size() * sizeof(PrimVertex), 
                         BufferUsage::StaticDraw, 
                         verts.data()),
          m_IndexBuffer((uint32_t)inds.size(),
                         IndexType::U32, 
                         BufferUsage::StaticDraw, 
                         inds.data()),
          m_IndexCount(static_cast<uint32_t>(inds.size()))
    {
        // set vertice layout for this mesh
        BufferLayout layout({
            { ShaderDataType::Float3, "a_Position",  (uint32_t)offsetof(PrimVertex, px) },
            { ShaderDataType::Float2, "a_TexCoord",  (uint32_t)offsetof(PrimVertex, u) },
            { ShaderDataType::Float3, "a_Normal",    (uint32_t)offsetof(PrimVertex, nx) },
            { ShaderDataType::Float4, "a_Tangent",   (uint32_t)offsetof(PrimVertex, tx) }
            }, (uint32_t)sizeof(PrimVertex));

        m_VertexBuffer.SetLayout(layout); // set vertex buffer layout
        m_VertexArray.AddVertexBuffer(m_VertexBuffer); // bind vertex buffer to VAO
        m_VertexArray.SetIndexBuffer(m_IndexBuffer); // bind index buffer to VAO
    }

    ~MeshGL() = default;
};