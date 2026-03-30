#include"Renderer/Renderer.h"
#include "Assets/Mesh/MeshManager.h"
#include "Assets/Material/MaterialManager.h"
#include "Assets/Texture/TextureManager.h"
#include "Assets/Shader/ShaderManager.h"
#include "Renderer/FramebufferManager.h"
#include <glad/gl.h>

Renderer::Renderer(ShaderManager& shaderMgr,
                   TextureManager& textureMgr,
                   MeshManager& meshMgr,
                   MaterialManager& materialMgr,
                   FramebufferManager& framebufferMgr)
    : m_ShaderMgr(shaderMgr)
    , m_TextureMgr(textureMgr)
    , m_MeshMgr(meshMgr)
    , m_MaterialMgr(materialMgr)
    , m_FramebufferMgr(framebufferMgr)
{
}

void Renderer::BeginFrame()
{
    m_OpaqueItems.clear();
    m_TransparentItems.clear();
    m_CurrentPass.reset();

    InvalidateBindCache();
}

void Renderer::EndFrame()
{
    CORE_ASSERT(!m_CurrentPass.has_value(), "EndFrame: a render pass is still active");
}

void Renderer::ReserveDrawItems(size_t opaqueCount, size_t transparentCount)
{
    m_OpaqueItems.reserve(opaqueCount);
    m_TransparentItems.reserve(transparentCount);
}

void Renderer::InvalidateBindCache()
{
    m_BoundFBO      = ~0u;
    m_BoundPipeline = ~0u;
    m_BoundVAO      = 0;

    m_MaterialMgr.InvalidateBindCache();
}

void Renderer::BindRenderTargetIfNeeded(const RenderTarget& target)
{
    u32 fbo = 0;

    if (target.m_Kind == RenderTargetKind::Framebuffer)
    {
        CORE_ASSERT(m_FramebufferMgr.IsValid(target.m_Framebuffer),
                    "BindRenderTargetIfNeeded: invalid framebuffer");
        fbo = m_FramebufferMgr.GetGLHandle(target.m_Framebuffer);
    }

    if (m_BoundFBO == fbo)
        return;

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    m_BoundFBO = fbo;

    // FBO switch invalidates some local state assumptions.
    m_BoundVAO = 0;
    m_MaterialMgr.InvalidateBindCache();
}

void Renderer::BeginPass(const RenderPassDesc& desc)
{
    CORE_ASSERT(!m_CurrentPass.has_value(), "BeginPass: previous pass not ended");

    m_CurrentPass = desc;
    m_OpaqueItems.clear();
    m_TransparentItems.clear();

    BindRenderTargetIfNeeded(desc.m_Target);

    glViewport(0,
               0,
               static_cast<GLsizei>(desc.m_ViewportWidth),
               static_cast<GLsizei>(desc.m_ViewportHeight));

    bool hasColor   = true;
    bool hasDepth   = true;
    bool hasStencil = false;

    if (desc.m_Target.m_Kind == RenderTargetKind::Framebuffer)
    {
        const RenderTargetDesc& rt = m_FramebufferMgr.GetDesc(desc.m_Target.m_Framebuffer);

        hasColor = !rt.m_ColorAttachments.empty();
        hasDepth = rt.m_DepthAttachment.has_value();
        hasStencil = rt.m_DepthAttachment.has_value() &&
                     rt.m_DepthAttachment->m_Kind == AttachmentKind::DepthStencil;
    }
    else
    {
        // Default backbuffer usually has color, and commonly depth/stencil as well.
        hasColor   = true;
        hasDepth   = true;
        hasStencil = true;
    }

    GLbitfield clearMask = 0;

    if (hasColor && desc.m_ColorLoadOp == LoadOp::Clear)
    {
        glClearColor(desc.m_ClearColor.x,
                     desc.m_ClearColor.y,
                     desc.m_ClearColor.z,
                     desc.m_ClearColor.w);
        clearMask |= GL_COLOR_BUFFER_BIT;
    }

    if (hasDepth && desc.m_DepthLoadOp == LoadOp::Clear)
    {
        glClearDepth(desc.m_ClearDepth);
        clearMask |= GL_DEPTH_BUFFER_BIT;

        if (hasStencil)
        {
            glClearStencil(desc.m_ClearStencil);
            clearMask |= GL_STENCIL_BUFFER_BIT;
        }
    }

    if (clearMask != 0)
        glClear(clearMask);

    // New pass => local caches should be treated as dirty.
    m_BoundPipeline = ~0u;
    m_BoundVAO      = 0;
    m_MaterialMgr.InvalidateBindCache();
}

void Renderer::EndPass()
{
    CORE_ASSERT(m_CurrentPass.has_value(), "EndPass: no active pass");

    const RenderPassDesc& pass = *m_CurrentPass;

    // Auto-generate mips for color attachments that requested it.
    if (pass.m_Target.m_Kind == RenderTargetKind::Framebuffer)
    {
        const FramebufferHandle fb = pass.m_Target.m_Framebuffer;
        const RenderTargetDesc& rt = m_FramebufferMgr.GetDesc(fb);

        for (size_t i = 0; i < rt.m_ColorAttachments.size(); ++i)
        {
            const AttachmentSpec& spec = rt.m_ColorAttachments[i];
            if (spec.m_MipPolicy == MipPolicy::AutoGenerate)
            {
                TextureHandle tex = m_FramebufferMgr.GetColorAttachment(fb, i);
                if (tex)
                    m_TextureMgr.GenerateMips(tex);
            }
        }
    }

    m_CurrentPass.reset();
}

void Renderer::SubmitOpaque(MeshHandle mesh,
                            MaterialHandle material,
                            const Mat4& model,
                            float depth,
                            PipelineKey pipeline)
{
    CORE_ASSERT(m_CurrentPass.has_value(), "SubmitOpaque: no active pass");
    CORE_ASSERT(m_MeshMgr.IsValid(mesh), "SubmitOpaque: invalid mesh");
    CORE_ASSERT(m_MaterialMgr.IsValid(material), "SubmitOpaque: invalid material");

    m_OpaqueItems.push_back(RenderItem{
        .m_Pipeline  = pipeline,
        .m_Mesh      = mesh,
        .m_Material  = material,
        .m_Shader    = m_MaterialMgr.GetShaderHandle(material),
        .m_ApplyHash = m_MaterialMgr.GetApplyHash(material),
        .m_Model     = model,
        .m_Depth     = depth
    });
}

void Renderer::SubmitTransparent(MeshHandle mesh,
                                 MaterialHandle material,
                                 const Mat4& model,
                                 float depth,
                                 PipelineKey pipeline)
{
    CORE_ASSERT(m_CurrentPass.has_value(), "SubmitTransparent: no active pass");
    CORE_ASSERT(m_MeshMgr.IsValid(mesh), "SubmitTransparent: invalid mesh");
    CORE_ASSERT(m_MaterialMgr.IsValid(material), "SubmitTransparent: invalid material");

    m_TransparentItems.push_back(RenderItem{
        .m_Pipeline  = pipeline,
        .m_Mesh      = mesh,
        .m_Material  = material,
        .m_Shader    = m_MaterialMgr.GetShaderHandle(material),
        .m_ApplyHash = m_MaterialMgr.GetApplyHash(material),
        .m_Model     = model,
        .m_Depth     = depth
    });
}

void Renderer::SubmitOpaqueBatch(std::span<const SubmitItem> items)
{
    CORE_ASSERT(m_CurrentPass.has_value(), "SubmitOpaqueBatch: no active pass");

    m_OpaqueItems.reserve(m_OpaqueItems.size() + items.size());

    for (const SubmitItem& item : items)
    {
        SubmitOpaque(
            item.m_Mesh,
            item.m_Material,
            item.m_Model,
            item.m_Depth,
            item.m_Pipeline
        );
    }
}

void Renderer::SubmitTransparentBatch(std::span<const SubmitItem> items)
{
    CORE_ASSERT(m_CurrentPass.has_value(), "SubmitTransparentBatch: no active pass");

    m_TransparentItems.reserve(m_TransparentItems.size() + items.size());

    for (const SubmitItem& item : items)
    {
        SubmitTransparent(
            item.m_Mesh,
            item.m_Material,
            item.m_Model,
            item.m_Depth,
            item.m_Pipeline
        );
    }
}

void Renderer::SortOpaqueItems()
{
    std::sort(m_OpaqueItems.begin(), m_OpaqueItems.end(),
        [](const RenderItem& a, const RenderItem& b)
        {
            if (a.m_Pipeline != b.m_Pipeline)
                return a.m_Pipeline < b.m_Pipeline;

            if (a.m_Shader.m_Id != b.m_Shader.m_Id)
                return a.m_Shader.m_Id < b.m_Shader.m_Id;
            if (a.m_Shader.m_Generation != b.m_Shader.m_Generation)
                return a.m_Shader.m_Generation < b.m_Shader.m_Generation;

            if (a.m_ApplyHash != b.m_ApplyHash)
                return a.m_ApplyHash < b.m_ApplyHash;

            if (a.m_Mesh.m_Id != b.m_Mesh.m_Id)
                return a.m_Mesh.m_Id < b.m_Mesh.m_Id;
            if (a.m_Mesh.m_Generation != b.m_Mesh.m_Generation)
                return a.m_Mesh.m_Generation < b.m_Mesh.m_Generation;

            return a.m_Depth < b.m_Depth; // front-to-back
        });
}

void Renderer::SortTransparentItems()
{
    std::sort(m_TransparentItems.begin(), m_TransparentItems.end(),
        [](const RenderItem& a, const RenderItem& b)
        {
            if (a.m_Depth != b.m_Depth)
                return a.m_Depth > b.m_Depth; // back-to-front

            if (a.m_Pipeline != b.m_Pipeline)
                return a.m_Pipeline < b.m_Pipeline;

            if (a.m_Shader.m_Id != b.m_Shader.m_Id)
                return a.m_Shader.m_Id < b.m_Shader.m_Id;
            if (a.m_Shader.m_Generation != b.m_Shader.m_Generation)
                return a.m_Shader.m_Generation < b.m_Shader.m_Generation;

            return a.m_ApplyHash < b.m_ApplyHash;
        });
}

void Renderer::BindPipelineIfNeeded(PipelineKey key)
{
    if (m_BoundPipeline == key)
        return;

    ApplyPipeline(key);
    m_BoundPipeline = key;
}

void Renderer::ApplyPipeline(PipelineKey key)
{
    const bool depthTest  = ((key >> 0) & 1u) != 0;
    const bool depthWrite = ((key >> 1) & 1u) != 0;

    const CullMode  cullMode  = static_cast<CullMode>((key >> 2) & 0x3u);
    const DepthFunc depthFunc = static_cast<DepthFunc>((key >> 4) & 0x3u);
    const BlendMode blendMode = static_cast<BlendMode>((key >> 6) & 0x7u);

    // Depth state
    if (depthTest) glEnable(GL_DEPTH_TEST);
    else           glDisable(GL_DEPTH_TEST);

    glDepthMask(depthWrite ? GL_TRUE : GL_FALSE);

    switch (depthFunc)
    {
    case DepthFunc::Less:
        glDepthFunc(GL_LESS);
        break;

    case DepthFunc::LessEqual:
        glDepthFunc(GL_LEQUAL);
        break;

    case DepthFunc::Equal:
        glDepthFunc(GL_EQUAL);
        break;

    case DepthFunc::Always:
        glDepthFunc(GL_ALWAYS);
        break;
    }

    // Cull state
    switch (cullMode)
    {
    case CullMode::None:
        glDisable(GL_CULL_FACE);
        break;

    case CullMode::Back:
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        break;

    case CullMode::Front:
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        break;
    }

    // Blend state
    switch (blendMode)
    {
    case BlendMode::Opaque:
        glDisable(GL_BLEND);
        break;

    case BlendMode::Alpha:
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        break;

    case BlendMode::PremultipliedAlpha:
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        break;

    case BlendMode::Additive:
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        break;

    case BlendMode::Multiply:
        glEnable(GL_BLEND);
        glBlendFunc(GL_DST_COLOR, GL_ZERO);
        break;

    default:
        CORE_ASSERT(false, "Unknown BlendMode");
        glDisable(GL_BLEND);
        break;
    }
}

void Renderer::BindMeshIfNeeded(MeshHandle h)
{
    const MeshGL& mesh = m_MeshMgr.GetGL(h);

    const u32 vao = mesh.m_VertexArray.GetRendererID();
    if (m_BoundVAO == vao)
        return;

    mesh.m_VertexArray.Bind();
    m_BoundVAO = vao;
}

ProgramUniformCache& Renderer::GetOrCreateProgramUniformCache(Shader& sh)
{
    const u32 prog = sh.GetProgramHandle();

    auto it = m_ProgramUniformCache.find(prog);
    if (it != m_ProgramUniformCache.end())
        return it->second;

    ProgramUniformCache cache{};
    cache.m_Program  = prog;
    cache.u_ViewProj = sh.GetUniformHandle("u_ViewProj");
    cache.u_Model    = sh.GetUniformHandle("u_Model");

    auto [iter, inserted] = m_ProgramUniformCache.emplace(prog, cache);
    return iter->second;
}

void Renderer::FlushOpaque(const Mat4& viewProj)
{
    CORE_ASSERT(m_CurrentPass.has_value(), "FlushOpaque: no active pass");

    SortOpaqueItems();

    u32 lastViewProjProgram = 0;

    for (const RenderItem& item : m_OpaqueItems)
    {
        BindPipelineIfNeeded(item.m_Pipeline);

        m_MaterialMgr.Bind(item.m_Material);

        Shader& sh = m_ShaderMgr.Get(item.m_Shader);
        const u32 prog = sh.GetProgramHandle();

        ProgramUniformCache& uc = GetOrCreateProgramUniformCache(sh);

        // Per-program / per-pass setup
        if (lastViewProjProgram != prog)
        {
            if (uc.u_ViewProj)
                sh.SetUniform(uc.u_ViewProj, viewProj);

            if (m_CurrentPass->m_GlobalsBinder)
                m_CurrentPass->m_GlobalsBinder.Bind(sh, m_TextureMgr);

            lastViewProjProgram = prog;
        }

        // Per-object setup
        if (uc.u_Model)
            sh.SetUniform(uc.u_Model, item.m_Model);

        BindMeshIfNeeded(item.m_Mesh);

        const MeshGL& mesh = m_MeshMgr.GetGL(item.m_Mesh);
        glDrawElements(GL_TRIANGLES, mesh.m_IndexCount, GL_UNSIGNED_INT, nullptr);
    }

    m_OpaqueItems.clear();
}

void Renderer::FlushTransparent(const Mat4& viewProj)
{
    CORE_ASSERT(m_CurrentPass.has_value(), "FlushTransparent: no active pass");

    SortTransparentItems();

    u32 lastViewProjProgram = 0;

    for (const RenderItem& item : m_TransparentItems)
    {
        BindPipelineIfNeeded(item.m_Pipeline);

        m_MaterialMgr.Bind(item.m_Material);

        Shader& sh = m_ShaderMgr.Get(item.m_Shader);
        const u32 prog = sh.GetProgramHandle();

        ProgramUniformCache& uc = GetOrCreateProgramUniformCache(sh);

        if (lastViewProjProgram != prog)
        {
            if (uc.u_ViewProj)
                sh.SetUniform(uc.u_ViewProj, viewProj);

            if (m_CurrentPass->m_GlobalsBinder)
                m_CurrentPass->m_GlobalsBinder.Bind(sh, m_TextureMgr);

            lastViewProjProgram = prog;
        }

        if (uc.u_Model)
            sh.SetUniform(uc.u_Model, item.m_Model);

        BindMeshIfNeeded(item.m_Mesh);

        const MeshGL& mesh = m_MeshMgr.GetGL(item.m_Mesh);
        glDrawElements(GL_TRIANGLES, mesh.m_IndexCount, GL_UNSIGNED_INT, nullptr);
    }

    m_TransparentItems.clear();
}