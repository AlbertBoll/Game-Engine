#include "DemoTriangleLayer.h"
#include "Assets/Material/MaterialDescBuilder.h"
#include "Assets/Material/MaterialRenderPolicy.h"
#include "Core/Application.h"
#include "Windows/Window.h"


void DemoTriangleLayer::OnAttach()
{
    m_TextureMgr.InitDefaultTextures();
    m_Renderer.ReserveDrawItems(16, 16);
    m_TriangleShader = m_ShaderMgr.GetOrCreateFromFiles({
        "Assets/Shader/ShaderSource/unlit_triangle.vert",
        "Assets/Shader/ShaderSource/unlit_triangle.frag"});
    m_TriangleMesh = m_MeshMgr.GetOrCreate(TriangleKey{}, "Triangle");
    m_TriangleMat = m_MaterialMgr.GetOrCreate(
            MaterialDescBuilder{}
                .Shader(m_TriangleShader)
                .OpaqueSurface()
                .Albedo(Vec3f(1.0f, 0.2f, 0.2f))
                .BuildChecked()
        );
    m_PassDesc.m_Pass = RenderPass::Opaque;
    m_PassDesc.m_Target = RenderTarget{
        .m_Kind = RenderTargetKind::BackBuffer
    };
    m_PassDesc.m_ColorLoadOp = LoadOp::Clear;
    m_PassDesc.m_DepthLoadOp = LoadOp::Clear;
    m_PassDesc.m_ClearColor = Vec4f(0.1f, 0.1f, 0.12f, 1.0f);
    m_PassDesc.m_ClearDepth = 1.0f;
    m_PassDesc.m_ClearStencil = 0;
}

void DemoTriangleLayer::OnRender()
{
    auto& window = Application::Get().GetWindow();
    const auto& props = window.GetWindowProperties();
    m_PassDesc.m_ViewportWidth  = props.m_Width;
    m_PassDesc.m_ViewportHeight = props.m_Height;

    const Mat4 identity(1.0f);

    m_Renderer.BeginFrame();
    m_Renderer.BeginPass(m_PassDesc);

    const MaterialDesc& md = m_MaterialMgr.GetDesc(m_TriangleMat);
    const MaterialSubmitInfo submitInfo = BuildSubmitInfo(md);

    m_Renderer.SubmitOpaque(
        m_TriangleMesh,
        m_TriangleMat,
        Mat4(1.0f),
        0.0f,
        submitInfo.m_Pipeline
    );

    m_Renderer.FlushOpaque(identity);
    // m_Renderer->FlushTransparent(identity);

    m_Renderer.EndPass();
    m_Renderer.EndFrame();
}
