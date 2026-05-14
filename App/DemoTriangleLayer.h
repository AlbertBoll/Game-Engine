#pragma once

#include "Core/Layer.h"
#include "Assets/Shader/ShaderManager.h"
#include "Assets/Texture/TextureManager.h"
#include "Assets/Mesh/MeshManager.h"
#include "Assets/Material/MaterialManager.h"
#include "Renderer/FramebufferManager.h"
#include "Renderer/Renderer.h"

class DemoTriangleLayer: public Layer
{

public:
    //DemoTriangleLayer() = default;
    DemoTriangleLayer(): Layer("Demo Layer"),
                         m_FramebufferMgr(m_TextureMgr),
                         m_Renderer(m_ShaderMgr, m_TextureMgr, m_MeshMgr, m_MaterialMgr, m_FramebufferMgr)
    {
        m_MaterialMgr.SetShaderManager(m_ShaderMgr);
        m_MaterialMgr.SetTextureManager(m_TextureMgr);
    }

    ~DemoTriangleLayer() override = default;

    void OnAttach() override;

    void OnUpdate(Timestep ts) override{}

    void OnRender() override;

                         

private:
    ShaderManager      m_ShaderMgr;
    TextureManager     m_TextureMgr;
    MeshManager        m_MeshMgr;
    MaterialManager    m_MaterialMgr;
    FramebufferManager m_FramebufferMgr;
    Renderer           m_Renderer;

    ShaderHandle   m_TriangleShader{};
    MeshHandle     m_TriangleMesh{};
    MaterialHandle m_TriangleMat{};

    RenderPassDesc m_PassDesc{};
};
