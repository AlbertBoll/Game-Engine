#pragma once

#include "Core/Base.h"
#include <unordered_map>
#include "Math/Math.h"
#include <string_view>
#include <vector>
#include "ShaderDesc.h"


using namespace Math;

enum class ShaderStage: uint8_t
{
    VERTEX,
    FRAGMENT,
    GEOMETRY,
    TESS_CONTROL,
    TESS_EVAL,
    COMPUTE
};

struct ShaderStageSource
{
    ShaderStage m_Stage{};
    std::string m_Source;
    std::string m_DebugName;
};

enum class TextureTarget : uint8_t 
{ 
    Tex2D, 
    Cube, 
    Tex2DArray, 
    Tex3D 
};

struct TextureBinding
{
    TextureTarget m_Target = TextureTarget::Tex2D;
    uint32_t m_TexId = 0;
    uint32_t m_Unit  = 0;
};

 // ---- transparent lookup for unordered_map<string, ...> using string_view ----
struct TransparentStringHash
{
    using is_transparent = void;
    size_t operator()(std::string_view sv) const noexcept { return std::hash<std::string_view>{}(sv); }
    size_t operator()(const std::string& s) const noexcept { return std::hash<std::string_view>{}(std::string_view{s}); }
};

struct TransparentStringEq
{
    using is_transparent = void;
    bool operator()(std::string_view a, std::string_view b) const noexcept { return a == b; }
    bool operator()(const std::string& a, std::string_view b) const noexcept { return std::string_view{a} == b; }
    bool operator()(std::string_view a, const std::string& b) const noexcept { return a == std::string_view{b}; }
    bool operator()(const std::string& a, const std::string& b) const noexcept { return a == b; }
};


class Shader
{
public:
    Shader() = default;
    ~Shader();

    MOVEONLY(Shader)

    // Build from files or sources
    void BuildFromFiles(std::span<const std::string> files); // infer stage from extension
    void BuildFromSources(std::span<const ShaderStageSource> sources);
    
    void Bind() const;
    void UnBind() const;

    [[nodiscard]] bool IsLinked() const noexcept { return m_Linked; }
    [[nodiscard]] uint32_t GetProgramHandle() const noexcept { return m_Program; }

    // --- Uniform handle (fast) ---
    UniformHandle GetUniformHandle(std::string_view name) const;

     // --- SetUniform by name (still fast: cached + string_view find) ---
    void SetUniform(std::string_view name, int v) const;
    void SetUniform(std::string_view name, unsigned v) const;
    void SetUniform(std::string_view name, float v) const;
    void SetUniform(std::string_view name, bool v) const;

    void SetUniform(std::string_view name, const Vec2f& v) const;
    void SetUniform(std::string_view name, const Vec3f& v) const;
    void SetUniform(std::string_view name, const Vec4f& v) const;

    void SetUniform(std::string_view name, const Vec2i& v) const;
    void SetUniform(std::string_view name, const Vec3i& v) const;
    void SetUniform(std::string_view name, const Vec4i& v) const;

    void SetUniform(std::string_view name, const Mat2& m, bool transpose=false) const;
    void SetUniform(std::string_view name, const Mat3& m, bool transpose=false) const;
    void SetUniform(std::string_view name, const Mat4& m, bool transpose=false) const;

    void SetUniform(std::string_view name, std::span<const Vec4f> v) const;
    void SetUniform(std::string_view name, std::span<const Vec3f> v) const;
    void SetUniform(std::string_view name, std::span<const Vec2f> v) const;

    void SetUniform(std::string_view name, std::span<const Mat4> m, bool transpose=false) const;

    // Low-level escape hatch only.
    // Main renderer/material path should prefer TextureManager::Bind(TextureHandle, unit)
    // and then set sampler uniform separately.
    void SetUniform(std::string_view name, const TextureBinding& tb) const;

    // --- SetUniform by handle (最热路径用这个) ---
    void SetUniform(UniformHandle h, int v) const;
    void SetUniform(UniformHandle h, float v) const;

    void SetUniform(UniformHandle h, const Vec2f& v) const;
    void SetUniform(UniformHandle h, const Vec3f& v) const;
    void SetUniform(UniformHandle h, const Vec4f& v) const;

    void SetUniform(UniformHandle h, const Mat4& m, bool transpose=false) const;
    void SetUniform(UniformHandle h, std::span<const Mat4> m, bool transpose=false) const;

    void SetUniform(UniformHandle h, std::span<const Vec2f> m) const;
    void SetUniform(UniformHandle h, std::span<const Vec3f> m) const;
    void SetUniform(UniformHandle h, std::span<const Vec4f> m) const;

    void SetUniform(UniformHandle h, std::span<const float> m) const;

    // Debug helpers (optional)
    void Validate() const;

private:
    void Destroy() noexcept;
    void MoveFrom(Shader& other) noexcept;

    void CreateProgramIfNeeded();
    void AttachAndCompile(ShaderStage stage, const std::string& src, std::string_view debugName);
    void LinkProgramOrThrow();
    void CacheActiveUniformLocations(); // high-performance core

    static ShaderStage InferStageFromPath(std::string_view path);
    static std::string ReadTextFile(const std::string& path);

    int32_t GetUniformLocationCached(std::string_view name) const;

private:
    uint32_t m_Program{0};
    bool m_Linked{false};

    // name -> location (cached after link)
    mutable std::unordered_map<std::string, int32_t, TransparentStringHash, TransparentStringEq> m_UniformLoc;

    // keep attached shader objects until link then delete
    std::vector<uint32_t> m_AttachedShaders;

};