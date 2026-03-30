#include"Assets/Shader/Shader.h"
#include<glad/gl.h>
#include<fstream>
#include<sstream>
#include <glm/gtc/type_ptr.hpp>

static GLenum ToGLStage(ShaderStage s)
{
    switch (s)
    {
        case ShaderStage::VERTEX:      return GL_VERTEX_SHADER;
        case ShaderStage::FRAGMENT:    return GL_FRAGMENT_SHADER;
        case ShaderStage::GEOMETRY:    return GL_GEOMETRY_SHADER;
        case ShaderStage::TESS_CONTROL: return GL_TESS_CONTROL_SHADER;
        case ShaderStage::TESS_EVAL:    return GL_TESS_EVALUATION_SHADER;
        case ShaderStage::COMPUTE:     return GL_COMPUTE_SHADER;
        default:                       return GL_VERTEX_SHADER;
    }
}

static GLenum ToGLTexTarget(TextureTarget t)
{
    switch (t)
    {
        case TextureTarget::Cube:       return GL_TEXTURE_CUBE_MAP;
        case TextureTarget::Tex2D:      return GL_TEXTURE_2D;
        case TextureTarget::Tex2DArray: return GL_TEXTURE_2D_ARRAY;
        case TextureTarget::Tex3D:      return GL_TEXTURE_3D;
        default:                        return GL_TEXTURE_2D;                    
    }
}

Shader::~Shader()
{
    Destroy();
}

Shader::Shader(Shader&& other) noexcept 
{
    MoveFrom(other);
}

Shader& Shader::operator=(Shader&& other) noexcept
{
    if(this != &other)
    {
        Destroy();
        MoveFrom(other);
    }
    return *this;
}

void Shader::CreateProgramIfNeeded()
{
    if (!m_Program)
    {
        m_Program = (uint32_t)glCreateProgram();
        CORE_ASSERT(m_Program != 0, "glCreateProgram failed");
    }
}


ShaderStage Shader::InferStageFromPath(std::string_view p)
{
    // extend extension
    auto ends_with = [&](std::string_view suf)
    {
        return p.size() >= suf.size() && p.substr(p.size() - suf.size()) == suf;
    };

    if (ends_with(".vs") || ends_with(".vert") || ends_with(".vert.glsl") || ends_with("_vert.glsl"))
        return ShaderStage::VERTEX;
    if (ends_with(".fs") || ends_with(".frag") || ends_with(".frag.glsl") || ends_with("_frag.glsl"))
        return ShaderStage::FRAGMENT;
    if (ends_with(".gs") || ends_with(".geom") || ends_with(".geom.glsl"))
        return ShaderStage::GEOMETRY;
    if (ends_with(".tcs") || ends_with(".tcs.glsl"))
        return ShaderStage::TESS_CONTROL;
    if (ends_with(".tes") || ends_with(".tes.glsl"))
        return ShaderStage::TESS_EVAL;
    if (ends_with(".cs") || ends_with(".cs.glsl"))
        return ShaderStage::COMPUTE;

    CORE_ASSERT(false, "Unrecognized shader extension");
    return ShaderStage::VERTEX;
}

void Shader::BuildFromFiles(std::span<const std::string> files)
{
    Destroy();
    CreateProgramIfNeeded();

    for(const auto& f: files)
    {
        ShaderStage st = InferStageFromPath(f);
        std::string src = ReadTextFile(f);
        AttachAndCompile(st, src, f);
    }

    LinkProgramOrThrow();
}

void Shader::AttachAndCompile(ShaderStage stage, const std::string& src, std::string_view debugName)
{
    CreateProgramIfNeeded();

    const GLuint sh = glCreateShader(ToGLStage(stage));
    CORE_ASSERT(sh != 0, "glCreateShader failed");

    const char* csrc = src.c_str();
    glShaderSource(sh, 1, &csrc, nullptr);
    glCompileShader(sh);

    GLint ok = 0;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        GLint len = 0;
        glGetShaderiv(sh, GL_INFO_LOG_LENGTH, &len);
        std::string log((size_t)len, '\0');
        if (len > 0) glGetShaderInfoLog(sh, len, nullptr, log.data());

        glDeleteShader(sh);

        // 你可以换成你自己的日志/异常系统
        CORE_ASSERT(false, (std::string("Shader compile failed: ") + std::string(debugName) + "\n" + log).c_str());
        return;
    }

    glAttachShader((GLuint)m_Program, sh);
    m_AttachedShaders.push_back((uint32_t)sh);
}

void Shader::LinkProgramOrThrow()
{
    CORE_ASSERT(m_Program != 0, "Program not created");
    glLinkProgram((GLuint)m_Program);

    GLint ok = 0;
    glGetProgramiv((GLuint)m_Program, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        GLint len = 0;
        glGetProgramiv((GLuint)m_Program, GL_INFO_LOG_LENGTH, &len);
        std::string log((size_t)len, '\0');
        if (len > 0) glGetProgramInfoLog((GLuint)m_Program, len, nullptr, log.data());

        CORE_ASSERT(false, (std::string("Program link failed:\n") + log).c_str());
        return;
    }

    // link 成功后：detach+delete attached shaders（program 已经包含最终结果）
    for (uint32_t sh : m_AttachedShaders)
    {
        glDetachShader((GLuint)m_Program, (GLuint)sh);
        glDeleteShader((GLuint)sh);
    }
    m_AttachedShaders.clear();

    // high performance：cache all active uniform locations
    CacheActiveUniformLocations();

    m_Linked = true;
}

std::string Shader::ReadTextFile(const std::string& path)
{
    std::ifstream in(path, std::ios::in);
    CORE_ASSERT((bool)in, "Shader file not found / can't open");
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

void Shader::Destroy() noexcept
{
    if(!m_AttachedShaders.empty())
    {
        for(uint32_t sh: m_AttachedShaders)
        {
            if(m_Program)
            {
                glDetachShader((GLuint)m_Program, (GLuint)sh);
                glDeleteShader((GLuint)sh);
            }
        }
        m_AttachedShaders.clear();
    }

    if(m_Program)
    {
        glDeleteProgram((GLuint)m_Program);
        m_Program = 0;
    }

    m_Linked = false;
    m_UniformLoc.clear();
}

void Shader::CacheActiveUniformLocations()
{
    m_UniformLoc.clear();

    GLint numUniforms = 0;
    glGetProgramInterfaceiv((GLuint)m_Program, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);

    // 预留，减少rehash
    m_UniformLoc.reserve((size_t)numUniforms * 2u + 16u);

    GLenum props[] = { GL_NAME_LENGTH, GL_LOCATION, GL_BLOCK_INDEX };

    std::string name;
    for (GLint i = 0; i < numUniforms; ++i)
    {
        GLint out[3]{};
        glGetProgramResourceiv((GLuint)m_Program, GL_UNIFORM, i, 3, props, 3, nullptr, out);

        const GLint nameLen = out[0];
        const GLint loc     = out[1];
        const GLint blkIdx  = out[2];

        if (blkIdx != -1) continue; // uniform block 里的跳过（UBO/SSBO用另一套）
        if (loc < 0) continue;      // optimized-out

        name.resize((size_t)nameLen);
        GLsizei written = 0;
        glGetProgramResourceName((GLuint)m_Program, GL_UNIFORM, i, nameLen, &written, name.data());

        if (!name.empty() && name.back() == '\0') name.pop_back();
        else name.resize((size_t)written);

        m_UniformLoc.emplace(name, (int32_t)loc);

        // 数组 base 名称：uBones[0] => uBones
        if (auto pos = name.find("[0]"); pos != std::string::npos)
        {
            std::string base = name.substr(0, pos);
            m_UniformLoc.emplace(std::move(base), (int32_t)loc);
        }
    }
}

void Shader::BuildFromSources(std::span<const ShaderStageSource> sources)
{
    Destroy();
    CreateProgramIfNeeded();

    for (const auto& s : sources)
        AttachAndCompile(s.m_Stage, s.m_Source, s.m_DebugName.empty() ? "source" : s.m_DebugName);

    LinkProgramOrThrow();
}

void Shader::Bind() const
{
    CORE_ASSERT(m_Linked && m_Program != 0, "Bind: program not linked");
    glUseProgram((GLuint)m_Program);
}

void Shader::UnBind() const
{
    glUseProgram(0);
}

void Shader::Validate() const
{
    CORE_ASSERT(m_Linked && m_Program != 0, "Validate: program not linked");
    glValidateProgram((GLuint)m_Program);
    GLint ok = 0;
    glGetProgramiv((GLuint)m_Program, GL_VALIDATE_STATUS, &ok);
    CORE_ASSERT(ok == GL_TRUE, "Program validate failed");
}


// 运行时查询：只做缓存查找（不再调用 glGetUniformLocation）
int32_t Shader::GetUniformLocationCached(std::string_view name) const
{
    if (auto it = m_UniformLoc.find(name); it != m_UniformLoc.end())
        return it->second;

    // miss：缓存 -1，避免反复查错名字造成热路径开销
    m_UniformLoc.emplace(std::string(name), -1);
    return -1;
}

UniformHandle Shader::GetUniformHandle(std::string_view name) const
{
    return UniformHandle{ GetUniformLocationCached(name) };
}

void Shader::MoveFrom(Shader& o) noexcept
{
    m_Program = o.m_Program;
    m_Linked  = o.m_Linked;
    m_UniformLoc = std::move(o.m_UniformLoc);
    m_AttachedShaders = std::move(o.m_AttachedShaders);

    o.m_Program = 0;
    o.m_Linked = false;
    o.m_UniformLoc.clear();
    o.m_AttachedShaders.clear();
}

void Shader::SetUniform(std::string_view name, int v) const
{
    const GLint loc = (GLint)GetUniformLocationCached(name);
    if(loc < 0) return;
    glUniform1i(loc, v);
}

void Shader::SetUniform(std::string_view name, unsigned v) const
{
    const GLint loc = (GLint)GetUniformLocationCached(name);
    if (loc < 0) return;
    glUniform1ui(loc, v);
}

void Shader::SetUniform(std::string_view name, float v) const
{
    const GLint loc = (GLint)GetUniformLocationCached(name);
    if (loc < 0) return;
    glUniform1f(loc, v);
}

 void Shader::SetUniform(std::string_view name, bool v) const
{
    const GLint loc = (GLint)GetUniformLocationCached(name);
    if (loc < 0) return;
    glUniform1i(loc, v ? 1 : 0);
}

void Shader::SetUniform(std::string_view name, const Vec2f& v) const
{
    const GLint loc = (GLint)GetUniformLocationCached(name);
    if (loc < 0) return;
    glUniform2fv(loc, 1, glm::value_ptr(v));
}

void Shader::SetUniform(std::string_view name, const Vec3f& v) const
{
    const GLint loc = (GLint)GetUniformLocationCached(name);
    if (loc < 0) return;
    glUniform3fv(loc, 1, glm::value_ptr(v));
}

void Shader::SetUniform(std::string_view name, const Vec4f& v) const
{
    const GLint loc = (GLint)GetUniformLocationCached(name);
    if (loc < 0) return;
    glUniform4fv(loc, 1, glm::value_ptr(v));
}

void Shader::SetUniform(std::string_view name, const Vec2i& v) const
{
    const GLint loc = (GLint)GetUniformLocationCached(name);
    if (loc < 0) return;
    glUniform2iv(loc, 1, &v.x);
}

void Shader::SetUniform(std::string_view name, const Vec3i& v) const
{
    const GLint loc = (GLint)GetUniformLocationCached(name);
    if (loc < 0) return;
    glUniform3iv(loc, 1, &v.x);
}

void Shader::SetUniform(std::string_view name, const Vec4i& v) const
{
    const GLint loc = (GLint)GetUniformLocationCached(name);
    if (loc < 0) return;
    glUniform4iv(loc, 1, &v.x);
}

void Shader::SetUniform(std::string_view name, const Mat2& m, bool transpose) const
{
    const GLint loc = (GLint)GetUniformLocationCached(name);
    if (loc < 0) return;
    glUniformMatrix2fv(loc, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(m));
}

void Shader::SetUniform(std::string_view name, const Mat3& m, bool transpose) const
{
    const GLint loc = (GLint)GetUniformLocationCached(name);
    if (loc < 0) return;
    glUniformMatrix3fv(loc, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(m));
}

void Shader::SetUniform(std::string_view name, const Mat4& m, bool transpose) const
{
    const GLint loc = (GLint)GetUniformLocationCached(name);
    if (loc < 0) return;
    glUniformMatrix4fv(loc, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(m));
}

void Shader::SetUniform(std::string_view name, std::span<const Vec3f> v) const
{
    const GLint loc = (GLint)GetUniformLocationCached(name);
    if (loc < 0 || v.empty()) return;
    glUniform3fv(loc, (GLsizei)v.size(), glm::value_ptr(v[0]));
}

void Shader::SetUniform(std::string_view name, std::span<const Vec2f> v) const
{
    const GLint loc = (GLint)GetUniformLocationCached(name);
    if (loc < 0 || v.empty()) return;
    glUniform2fv(loc, (GLsizei)v.size(), glm::value_ptr(v[0]));
}

void Shader::SetUniform(std::string_view name, std::span<const Vec4f> v) const
{
    const GLint loc = (GLint)GetUniformLocationCached(name);
    if (loc < 0 || v.empty()) return;
    glUniform4fv(loc, (GLsizei)v.size(), glm::value_ptr(v[0]));
}

void Shader::SetUniform(std::string_view name, std::span<const Mat4> m, bool transpose) const
{
    const GLint loc = (GLint)GetUniformLocationCached(name);
    if (loc < 0 || m.empty()) return;
    glUniformMatrix4fv(loc, (GLsizei)m.size(), transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(m[0]));
}



void Shader::SetUniform(std::string_view name, const TextureBinding& tb) const
{
    const GLint loc = (GLint)GetUniformLocationCached(name);
    if (loc < 0) return;

    glActiveTexture(GL_TEXTURE0 + tb.m_Unit);
    glBindTexture(ToGLTexTarget(tb.m_Target), (GLuint)tb.m_TexId);
    glUniform1i(loc, (GLint)tb.m_Unit);
}

// ---- SetUniform by handle (最热路径) ----
void Shader::SetUniform(UniformHandle h, int v) const
{
    if (!h) return;
    glUniform1i(h.m_Loc, v);
}

void Shader::SetUniform(UniformHandle h, float v) const
{
    if (!h) return;
    glUniform1f(h.m_Loc, v);
}

void Shader::SetUniform(UniformHandle h, const Vec3f& v) const
{
    if (!h) return;
    glUniform3fv(h.m_Loc, 1, glm::value_ptr(v));
}

void Shader::SetUniform(UniformHandle h, const Vec2f& v) const
{
    if (!h) return;
    glUniform2fv(h.m_Loc, 1, glm::value_ptr(v));
}

void Shader::SetUniform(UniformHandle h, const Vec4f& v) const
{
    if (!h) return;
    glUniform4fv(h.m_Loc, 1, glm::value_ptr(v));
}

void Shader::SetUniform(UniformHandle h, std::span<const Vec4f> v) const
{
    if (!h || v.empty()) return;
    glUniform4fv(h.m_Loc, (GLsizei)v.size(), glm::value_ptr(v[0]));
}

void Shader::SetUniform(UniformHandle h, std::span<const Vec3f> v) const
{
    if (!h|| v.empty()) return;
    glUniform3fv(h.m_Loc, (GLsizei)v.size(), glm::value_ptr(v[0]));
}

void Shader::SetUniform(UniformHandle h, std::span<const Vec2f> v) const
{
    if (!h|| v.empty()) return;
    glUniform2fv(h.m_Loc, (GLsizei)v.size(), glm::value_ptr(v[0]));
}

void Shader::SetUniform(UniformHandle h, const Mat4& m, bool transpose) const
{
    if (!h) return;
    glUniformMatrix4fv(h.m_Loc, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(m));
}

void Shader::SetUniform(UniformHandle h, std::span<const Mat4> m, bool transpose) const
{
    if (!h || m.empty()) return;
    glUniformMatrix4fv(h.m_Loc, (GLsizei)m.size(), transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(m[0]));
}

void Shader::SetUniform(UniformHandle h, std::span<const float> m) const
{
    if (!h || m.empty()) return;
    //glUniform3fv((GLint)h.m_Loc, (GLsizei)(m.size() / 3), m.data());
    glUniform1fv((GLint)h.m_Loc, (GLsizei)m.size(), m.data());
}



