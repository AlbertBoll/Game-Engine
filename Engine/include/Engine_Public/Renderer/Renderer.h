#pragma once

#include "Math/Math.h"
#include "Renderer/RenderTarget.h"
#include "Assets/Mesh/MeshDesc.h"
#include "Assets/Material/MaterialDesc.h"
#include "Assets/Shader/ShaderDesc.h"
#include <unordered_map>
#include "Renderer/PassGlobalsBinderRef.h"
#include <span>

using namespace Math;





class ShaderManager;
class TextureManager;
class MeshManager;
class MaterialManager;
class Shader;
class FramebufferManager;

enum class RenderPass : u8
{
    Shadow,
    Opaque,
    Skybox,
    Transparent,
    UI,
    PostProcessing
};

enum class CullMode : u8
{
    None,
    Back,
    Front
};

enum class DepthFunc : u8
{
    Less,
    LessEqual,
    Equal,
    Always
};

enum class BlendMode: u8
{
    Opaque = 0,              // blending disabled
    Alpha,                   // srcAlpha, oneMinusSrcAlpha
    PremultipliedAlpha,      // one, oneMinusSrcAlpha
    Additive,                // one, one
    Multiply                 // dstColor, zero

};

struct PipelineState
{
    bool      b_DepthTest  = true;
    bool      b_DepthWrite = true;
    bool      b_Blending   = false;
    CullMode  m_CullMode   = CullMode::Back;
    DepthFunc m_DepthFunc  = DepthFunc::Less;
    BlendMode m_BlendMode  = BlendMode::Opaque;
};

using PipelineKey = u32;

// Bit layout:
// bit 0      : depth test
// bit 1      : depth write
// bit 2~3    : cull mode
// bit 4~5    : depth func
// bit 6~8    : blend mode
inline PipelineKey BuildPipelineKey(const PipelineState& p)
{
    PipelineKey k = 0;

    k |= (p.b_DepthTest  ? 1u : 0u) << 0;
    k |= (p.b_DepthWrite ? 1u : 0u) << 1;
    //k |= (p.b_Blending   ? 1u : 0u) << 2;

    // bit 3~4: cull mode
    k |= (static_cast<u32>(p.m_CullMode)  & 0x3u) << 2;
    // bit 5~6: depth func
    k |= (static_cast<u32>(p.m_DepthFunc) & 0x3u) << 4;
    // bit 6~8: blend mode
    k |= (static_cast<u32>(p.m_BlendMode) & 0x7u) << 6;

    return k;
}

struct RenderPassDesc
{
    RenderPass   m_Pass = RenderPass::Opaque;
    RenderTarget m_Target{};

    u32 m_ViewportWidth  = 1;
    u32 m_ViewportHeight = 1;

    LoadOp  m_ColorLoadOp   = LoadOp::Clear;
    StoreOp m_ColorStoreOp  = StoreOp::Store;

    LoadOp  m_DepthLoadOp   = LoadOp::Clear;
    StoreOp m_DepthStoreOp  = StoreOp::Store;

    Vec4f m_ClearColor{0.0f, 0.0f, 0.0f, 1.0f};
    float m_ClearDepth   = 1.0f;
    i32   m_ClearStencil = 0;

    // Non-owning binder reference.
    // The referenced binder object must outlive BeginPass(...) ~ EndPass(...).
    PassGlobalsBinderRef m_GlobalsBinder{};
};

// ------------------------------------------------------------
// External submit payload
// ------------------------------------------------------------
struct SubmitItem
{
    MeshHandle     m_Mesh{};
    MaterialHandle m_Material{};
    Mat4           m_Model{1.0f};
    float          m_Depth = 0.0f;
    PipelineKey    m_Pipeline = 0;
};


struct RenderItem
{
    PipelineKey    m_Pipeline  = 0;

    MeshHandle     m_Mesh{};
    MaterialHandle m_Material{};
    ShaderHandle   m_Shader{};
    u64            m_ApplyHash = 0;

    Mat4           m_Model{1.0f};
    float          m_Depth = 0.0f;
};

struct ProgramUniformCache
{
    u32 m_Program = 0;
    UniformHandle u_ViewProj{};
    UniformHandle u_Model{};
};

class Renderer
{
public:
    Renderer(ShaderManager& shaderMgr,
             TextureManager& textureMgr,
             MeshManager& meshMgr,
             MaterialManager& materialMgr,
             FramebufferManager& framebufferManager);

    DELETE_COPY(Renderer);

    // --------------------------------------------------------
    // Frame
    // --------------------------------------------------------
    void BeginFrame();
    void EndFrame();

    // --------------------------------------------------------
    // Pass
    // --------------------------------------------------------
    void BeginPass(const RenderPassDesc& desc);
    void EndPass();

    // --------------------------------------------------------
    // Submit
    // --------------------------------------------------------
    void SubmitOpaque(MeshHandle mesh,
                      MaterialHandle material,
                      const Mat4& model,
                      float depth,
                      PipelineKey pipeline);

    void SubmitTransparent(MeshHandle mesh,
                           MaterialHandle material,
                           const Mat4& model,
                           float depth,
                           PipelineKey pipeline);

    void SubmitOpaqueBatch(std::span<const SubmitItem> items);
    void SubmitTransparentBatch(std::span<const SubmitItem> items);

    // --------------------------------------------------------
    // Flush
    // --------------------------------------------------------
    void FlushOpaque(const Mat4& viewProj);
    void FlushTransparent(const Mat4& viewProj);

    // --------------------------------------------------------
    // Capacity hint
    // --------------------------------------------------------
    void ReserveDrawItems(size_t opaqueCount, size_t transparentCount);

    // --------------------------------------------------------
    // External invalidation
    // --------------------------------------------------------
    void InvalidateBindCache();

private:
    void SortOpaqueItems();
    void SortTransparentItems();

    void BindRenderTargetIfNeeded(const RenderTarget& target);
    void BindPipelineIfNeeded(PipelineKey key);
    void ApplyPipeline(PipelineKey key);

    void BindMeshIfNeeded(MeshHandle h);

    ProgramUniformCache& GetOrCreateProgramUniformCache(Shader& sh);

private:
    ShaderManager&   m_ShaderMgr;
    TextureManager&  m_TextureMgr;
    MeshManager&     m_MeshMgr;
    MaterialManager& m_MaterialMgr;
    FramebufferManager& m_FramebufferMgr;

    std::vector<RenderItem> m_OpaqueItems;
    std::vector<RenderItem> m_TransparentItems;

    std::unordered_map<u32, ProgramUniformCache> m_ProgramUniformCache;
    
    std::optional<RenderPassDesc> m_CurrentPass{};

    PipelineKey m_BoundPipeline = ~0u;
    u32         m_BoundVAO      = 0;
    u32         m_BoundFBO      = ~0u;
};