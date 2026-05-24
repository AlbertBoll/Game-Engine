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

    auto& window = Application::Get().GetWindow();
    //const auto& props = window.GetWindowProperties();
    const WindowDrawableSize drawable = window.GetDrawableSize();

    RecreateSceneFramebuffer(drawable.m_Width, drawable.m_Height);
    // m_PassDesc.m_Pass = RenderPass::Opaque;
    // m_PassDesc.m_Target = RenderTarget{
    //     .m_Kind = RenderTargetKind::BackBuffer
    // };
    // m_PassDesc.m_ColorLoadOp = LoadOp::Clear;
    // m_PassDesc.m_DepthLoadOp = LoadOp::Clear;
    // m_PassDesc.m_ClearColor = Vec4f(0.1f, 0.1f, 0.12f, 1.0f);
    // m_PassDesc.m_ClearDepth = 1.0f;
    // m_PassDesc.m_ClearStencil = 0;
}

void DemoTriangleLayer::RecreateSceneFramebuffer(u32 width, u32 height)
{
    if (width == 0 || height == 0)
        return;

    if (m_SceneMsaaFB)
    {
        m_FramebufferMgr.Destroy(m_SceneMsaaFB);
        m_SceneMsaaFB = {};
    }

    RenderTargetDesc rt{};
    rt.m_Width   = width;
    rt.m_Height  = height;
    rt.m_Samples = TextureSampleCount::x4;

    AttachmentSpec color{};
    color.m_Kind      = AttachmentKind::Color;
    color.m_Format    = TextureFormat::RGBA8;
    color.m_MipPolicy = MipPolicy::OneLevel;
    rt.m_ColorAttachments.push_back(color);

    AttachmentSpec depth{};
    depth.m_Kind      = AttachmentKind::DepthStencil;
    depth.m_Format    = TextureFormat::Depth24Stencil8;
    depth.m_MipPolicy = MipPolicy::OneLevel;
    rt.m_DepthAttachment = depth;

    m_SceneMsaaFB = m_FramebufferMgr.CreateFromDesc(rt, "DemoTriangle_MSAA");

    m_FramebufferWidth  = width;
    m_FramebufferHeight = height;

    m_PassDesc = {};
    m_PassDesc.m_Pass = RenderPass::Opaque;

    m_PassDesc.m_Target = RenderTarget{
        .m_Kind = RenderTargetKind::Framebuffer,
        .m_Framebuffer = m_SceneMsaaFB
    };

    m_PassDesc.m_ViewportWidth  = width;
    m_PassDesc.m_ViewportHeight = height;

    m_PassDesc.m_ColorLoadOp = LoadOp::Clear;
    m_PassDesc.m_DepthLoadOp = LoadOp::Clear;

    m_PassDesc.m_ClearColor   = Vec4f(0.1f, 0.1f, 0.12f, 1.0f);
    m_PassDesc.m_ClearDepth   = 1.0f;
    m_PassDesc.m_ClearStencil = 0;
}

void DemoTriangleLayer::OnRender()
{
    auto& window = Application::Get().GetWindow();
    //const auto& props = window.GetWindowProperties();
    const WindowDrawableSize drawable = window.GetDrawableSize();

    const u32 width  = drawable.m_Width;
    const u32 height = drawable.m_Height;

    if (width == 0 || height == 0)
        return;

    if (!m_SceneMsaaFB ||
        width != m_FramebufferWidth ||
        height != m_FramebufferHeight)
    {
        RecreateSceneFramebuffer(width, height);
    }

    
    m_PassDesc.m_Target.m_Kind = RenderTargetKind::Framebuffer;
    m_PassDesc.m_Target.m_Framebuffer = m_SceneMsaaFB;
    m_PassDesc.m_ViewportWidth  = width;
    m_PassDesc.m_ViewportHeight = height;

    const Mat4 identity(1.0f);

    const MaterialDesc& md = m_MaterialMgr.GetDesc(m_TriangleMat);
    const MaterialSubmitInfo submitInfo = BuildSubmitInfo(md);

    m_Renderer.BeginFrame();

    m_Renderer.BeginPass(m_PassDesc);

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

    //const WindowDrawableSize drawable = window.GetDrawableSize();
    // MSAA framebuffer cannot be presented directly.
    // Resolve color attachment 0 into default framebuffer.
    m_Renderer.ResolveColorToBackBuffer(m_SceneMsaaFB, 0, width, height);

    m_Renderer.EndFrame();
}
