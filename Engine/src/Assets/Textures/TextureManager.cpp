#include "Assets/Texture/TextureManager.h"
#include "Core/Base.h"
#include <cstring>
#include <cmath>
#include <algorithm>
#include <glad/gl.h>
#include <stb_image.h>


// static TextureHandle CreateSolidRGBA8(TextureManager& tm,
//                                       u8 r, u8 g, u8 b, u8 a,
//                                       std::string_view debugName)
// {
//     const std::array<u8, 4> pixel{ r, g, b, a };

//     TextureDesc desc{};
//     desc.m_Type         = TextureType::Tex2D;
//     desc.m_Format       = TextureFormat::RGBA8;
//     desc.m_Width        = 1;
//     desc.m_Height       = 1;
//     desc.m_MipLevels    = 1;
//     desc.b_GenerateMips = false;
//     desc.m_Sampler      = SamplerDesc{};

//     return tm.CreateFromPixels2D(
//         desc,
//         std::span<const std::byte>(
//             reinterpret_cast<const std::byte*>(pixel.data()),
//             sizeof(pixel)),
//         0,
//         debugName
//     );
// }

// ---------------- path helpers ----------------
static inline std::string NormalizePath(std::string_view p)
{
    namespace fs = std::filesystem;
    fs::path pp{ std::string(p) };
    return pp.lexically_normal().generic_string();
}


// ---------------- validation ----------------
static void ValidateTextureDesc(const TextureDesc& d)
{
    CORE_ASSERT(d.m_Width > 0 && d.m_Height > 0, "TextureDesc width/height must be > 0");

    if (d.m_Type == TextureType::Cube)
        CORE_ASSERT(d.m_Width == d.m_Height, "Cube texture must be square");

    CORE_ASSERT(d.m_Sampler.m_MaxAniso >= 1.0f, "Sampler max anisotropy must be >= 1.0f");
}

// ---------------- mip utils ----------------
static inline u32 CalcFullMipLevels(u32 w, u32 h)
{
    u32 levels = 1;
    while (w > 1 || h > 1)
    {
        w = (w > 1) ? (w >> 1) : 1;
        h = (h > 1) ? (h >> 1) : 1;
        ++levels;
    }
    return levels;
}

static inline u32 EffectiveMipLevels(const TextureDesc& d)
{
    u32 m = (d.m_MipLevels == 0) ? CalcFullMipLevels(d.m_Width, d.m_Height) : (u32)d.m_MipLevels;
    return std::max<u32>(1, m);
}

// ---------------- safe delete ----------------
static inline void GLDeleteTex(u32& id)
{
    if (!id) return;
    static_assert(sizeof(GLuint) == sizeof(u32), "GLuint not 32-bit");
    GLuint tmp = (GLuint)id;
    glDeleteTextures(1, &tmp);
    id = 0;
}

static inline void GLDeleteSampler(u32& id)
{
    if (!id) return;
    static_assert(sizeof(GLuint) == sizeof(u32), "GLuint not 32-bit");
    GLuint tmp = (GLuint)id;
    glDeleteSamplers(1, &tmp);
    id = 0;
}

static inline void SetDebugLabel(GLenum identifier, u32 id, std::string_view name)
{
    if (name.empty()) return;
    if (GLAD_GL_VERSION_4_3 || GLAD_GL_KHR_debug)
        glObjectLabel(identifier, (GLuint)id, (GLsizei)name.size(), name.data());
}

// ---------------- PixelStore guard ----------------
struct PixelStoreGuard
{
    GLint m_Align  = 0;
    GLint m_RowLen = 0;

    PixelStoreGuard()
    {
        glGetIntegerv(GL_UNPACK_ALIGNMENT, &m_Align);
        glGetIntegerv(GL_UNPACK_ROW_LENGTH, &m_RowLen);
    }
    ~PixelStoreGuard()
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, m_Align);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, m_RowLen);
    }
};

// ---------------- external GL state guard ----------------
struct GLStateGuard
{
    TextureManager& m_TM;

    GLint m_PrevActiveTexEnum = 0;
    GLint m_PrevBind2D        = 0;
    GLint m_PrevBindCube      = 0;
    GLint m_PrevSampler0      = 0;

    GLStateGuard(TextureManager& tm)
        : m_TM(tm)
    {
        //m_TM.EnsureUnitCacheInit();

        glGetIntegerv(GL_ACTIVE_TEXTURE, &m_PrevActiveTexEnum);

        glActiveTexture(GL_TEXTURE0);
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_PrevBind2D);
        glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &m_PrevBindCube);
        glGetIntegeri_v(GL_SAMPLER_BINDING, 0, &m_PrevSampler0);
    }

    ~GLStateGuard()
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, (GLuint)m_PrevBind2D);
        glBindTexture(GL_TEXTURE_CUBE_MAP, (GLuint)m_PrevBindCube);
        glBindSampler(0, (GLuint)m_PrevSampler0);

        glActiveTexture((GLenum)m_PrevActiveTexEnum);

        // we recover external openGL state，so manager cache must invalidate
        m_TM.InvalidateBindCache();
    }
};

// ---------------- CPU flip Y ----------------
static void FlipY_U8_InPlace(unsigned char* data, int w, int h, int bpp)
{
    const int rowBytes = w * bpp;
    std::vector<unsigned char> tmp((size_t)rowBytes);

    for (int y = 0; y < h / 2; ++y)
    {
        unsigned char* r0 = data + (size_t)y * rowBytes;
        unsigned char* r1 = data + (size_t)(h - 1 - y) * rowBytes;
        std::memcpy(tmp.data(), r0, (size_t)rowBytes);
        std::memcpy(r0, r1, (size_t)rowBytes);
        std::memcpy(r1, tmp.data(), (size_t)rowBytes);
    }
}

static void FlipY_F32_InPlace(float* data, int w, int h, int channels)
{
    const int rowFloats = w * channels;
    std::vector<float> tmp((size_t)rowFloats);

    for (int y = 0; y < h / 2; ++y)
    {
        float* r0 = data + (size_t)y * rowFloats;
        float* r1 = data + (size_t)(h - 1 - y) * rowFloats;
        std::memcpy(tmp.data(), r0, (size_t)rowFloats * sizeof(float));
        std::memcpy(r0, r1, (size_t)rowFloats * sizeof(float));
        std::memcpy(r1, tmp.data(), (size_t)rowFloats * sizeof(float));
    }
}


// ---------------- format mapping ----------------
struct GLFormat
{
    GLenum m_InternalFmt = GL_RGBA8;
    GLenum m_DataFmt     = GL_RGBA;
    GLenum m_DataType    = GL_UNSIGNED_BYTE;
    u32    m_Bpp         = 4;
};

static GLFormat ToGL(TextureFormat fmt)
{
    switch (fmt)
    {
    case TextureFormat::R8:              return { GL_R8,               GL_RED,            GL_UNSIGNED_BYTE, 1 };
    case TextureFormat::RG8:             return { GL_RG8,              GL_RG,             GL_UNSIGNED_BYTE, 2 };
    case TextureFormat::RGB8:            return { GL_RGB8,             GL_RGB,            GL_UNSIGNED_BYTE, 3 };
    case TextureFormat::RGBA8:           return { GL_RGBA8,            GL_RGBA,           GL_UNSIGNED_BYTE, 4 };
    case TextureFormat::SRGB8:           return { GL_SRGB8,            GL_RGB,            GL_UNSIGNED_BYTE, 3 };
    case TextureFormat::SRGB8A8:         return { GL_SRGB8_ALPHA8,     GL_RGBA,           GL_UNSIGNED_BYTE, 4 };

    case TextureFormat::R16F:            return { GL_R16F,             GL_RED,            GL_FLOAT,         4 };
    case TextureFormat::RG16F:           return { GL_RG16F,            GL_RG,             GL_FLOAT,         8 };
    case TextureFormat::RGB16F:          return { GL_RGB16F,           GL_RGB,            GL_FLOAT,        12 };
    case TextureFormat::RGBA16F:         return { GL_RGBA16F,          GL_RGBA,           GL_FLOAT,        16 };

    case TextureFormat::Depth24Stencil8: return { GL_DEPTH24_STENCIL8,   GL_DEPTH_STENCIL,     GL_UNSIGNED_INT_24_8, 4 };
    case TextureFormat::Depth32F:        return { GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT,   GL_FLOAT,             4 };
    default: break;
    }
    return { GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, 4 };
}

static GLenum ToGLMinFilter(MinFilter f)
{
    switch(f)
    {
        case MinFilter::Nearest: return GL_NEAREST;
        case MinFilter::Linear: return GL_LINEAR;
        case MinFilter::NearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
        case MinFilter::LinearMipmapLinear:   return GL_LINEAR_MIPMAP_LINEAR;
    }
    return GL_LINEAR;
}

static GLenum ToGLMagFilter(MagFilter f)
{
    switch (f)
    {
    case MagFilter::Nearest: return GL_NEAREST;
    case MagFilter::Linear:  return GL_LINEAR;
    }
    return GL_LINEAR;
}

static GLenum ToGLWrap(Wrap w)
{
    switch (w)
    {
    case Wrap::Repeat:         return GL_REPEAT;
    case Wrap::ClampToEdge:    return GL_CLAMP_TO_EDGE;
    case Wrap::MirroredRepeat: return GL_MIRRORED_REPEAT;
    }
    return GL_REPEAT;
}

// ---------------- SamplerCache (OpenGL 4.3 core path) ----------------
struct TextureManager::SamplerCache
{
    std::unordered_map<SamplerDesc, u32, CustomHash<SamplerDesc>> m_Map;

    u32 GetOrCreate(const SamplerDesc& sp)
    {
        if (auto it = m_Map.find(sp); it != m_Map.end())
            return it->second;

        GLuint sid = 0;
        glGenSamplers(1, &sid);
        CORE_ASSERT(sid != 0, "glGenSamplers failed");

        glSamplerParameteri(sid, GL_TEXTURE_MIN_FILTER, (GLint)ToGLMinFilter(sp.m_MinFilter));
        glSamplerParameteri(sid, GL_TEXTURE_MAG_FILTER, (GLint)ToGLMagFilter(sp.m_MagFilter));
        glSamplerParameteri(sid, GL_TEXTURE_WRAP_S, (GLint)ToGLWrap(sp.m_WrapS));
        glSamplerParameteri(sid, GL_TEXTURE_WRAP_T, (GLint)ToGLWrap(sp.m_WrapT));
        glSamplerParameteri(sid, GL_TEXTURE_WRAP_R, (GLint)ToGLWrap(sp.m_WrapR));

        if (sp.b_EnableAniso && GLAD_GL_EXT_texture_filter_anisotropic)
        {
            GLfloat largest = 1.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest);
            float v = (sp.m_MaxAniso <= 1.0f) ? 1.0f : std::min(sp.m_MaxAniso, (float)largest);
            glSamplerParameterf(sid, GL_TEXTURE_MAX_ANISOTROPY_EXT, v);
        }

        m_Map.emplace(sp, (u32)sid);
        return (u32)sid;
    }

    void DestroyAll()
    {
        for (auto& kv : m_Map)
            GLDeleteSampler(kv.second);
        m_Map.clear();
    }
};

// ---------------- mip completeness ----------------
static void SetMipRange(GLenum target, u32 mipLevels)
{
    const GLint maxLevel = (GLint)std::max<u32>(1, mipLevels) - 1;
    glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, maxLevel);
}

// ---------------- choose formats ----------------
static TextureFormat ChooseLDRFormatByChannels(int comp, ColorSpace cs)
{
    CORE_ASSERT(comp >= 1 && comp <= 4, "Unsupported LDR channel count");

    if (comp == 1) return TextureFormat::R8;
    if (comp == 2) return TextureFormat::RG8;

    const bool wantSRGB = (cs == ColorSpace::ForceSRGB) || (cs == ColorSpace::Auto);

    if (comp == 3) return wantSRGB ? TextureFormat::SRGB8   : TextureFormat::RGB8;
    return wantSRGB ? TextureFormat::SRGB8A8 : TextureFormat::RGBA8;
}

static TextureFormat ChooseHDRFormatByChannels(int comp)
{
    CORE_ASSERT(comp >= 1 && comp <= 4, "Unsupported HDR channel count");

    switch (comp)
    {
    case 1: return TextureFormat::R16F;
    case 2: return TextureFormat::RG16F;
    case 3: return TextureFormat::RGB16F;
    case 4: return TextureFormat::RGBA16F;
    default: break;
    }

    return TextureFormat::RGBA16F;
}

// ---------------- fallback checker ----------------
static TextureHandle MakeChecker(TextureManager& tm, const SamplerDesc& sampler)
{
    const u32 checker[4] =
    {
        0xffffffffu, 0xff000000u,
        0xff000000u, 0xffffffffu
    };

    TextureDesc d{};
    d.m_Type         = TextureType::Tex2D;
    d.m_Width        = 2;
    d.m_Height       = 2;
    d.m_Format       = TextureFormat::RGBA8;
    d.m_Sampler      = sampler;
    d.m_MipLevels    = 1;
    d.b_GenerateMips = false;

    return tm.CreateFromPixels2D(
        d,
        std::span<const std::byte>((const std::byte*)checker, sizeof(checker)),
        0,
        "checker");
}

// ---------------- TextureKeyHash ----------------
std::size_t TextureKeyHash::operator()(const TextureKey& k) const noexcept
{
    return Combine(
            k.m_Path,
            k.b_IsCube,
            k.m_Range,
            k.m_ColorSpace,
            k.b_FlipY,
            k.m_Sampler
    );
    // std::size_t h = 0;
    // h = HashCombine(h, std::hash<std::string>{}(k.m_Path));
    // h = HashCombine(h, std::hash<bool>{}(k.b_IsCube));
    // h = HashCombine(h, std::hash<int>{}((int)k.m_Range));
    // h = HashCombine(h, std::hash<int>{}((int)k.m_ColorSpace));
    // h = HashCombine(h, std::hash<bool>{}(k.b_FlipY));
    // h = HashCombine(h, SamplerDescHash{}(k.m_Sampler));
    // return h;
}

// ---------------- TextureManager ----------------
TextureManager::~TextureManager()
{
    for (auto& s : m_Slots)
    {
        if (s.b_Alive)
        {
            GLDeleteTex(s.m_GLTex);
            s.b_Alive = false;
        }
    }

    DestroySamplerCache();
}

void TextureManager::DestroySamplerCache()
{
    if (!m_SamplerCache) return;
    m_SamplerCache->DestroyAll();
    delete m_SamplerCache;
    m_SamplerCache = nullptr;
}

u32 TextureManager::GetOrCreateSampler(const SamplerDesc& s)
{
    if (!m_SamplerCache) m_SamplerCache = new SamplerCache();
    return m_SamplerCache->GetOrCreate(s);
}

void TextureManager::EnsureUnitCacheInit() const
{
    CORE_ASSERT(GLAD_GL_VERSION_4_3, "TextureManager requires OpenGL 4.3 core");

    if (!m_BoundTex.empty()) return;

    GLint maxUnits = 0;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxUnits);
    if (maxUnits <= 0) maxUnits = 16;

    m_BoundTex.assign((size_t)maxUnits, 0);
    m_BoundSampler.assign((size_t)maxUnits, 0);
    m_ActiveUnit = 0xFFFFFFFFu;
}

void TextureManager::InvalidateBindCache() const
{
    if (!m_BoundTex.empty())
        std::fill(m_BoundTex.begin(), m_BoundTex.end(), 0);

    if (!m_BoundSampler.empty())
        std::fill(m_BoundSampler.begin(), m_BoundSampler.end(), 0);

    m_ActiveUnit = 0xFFFFFFFFu;
}

TextureHandle TextureManager::LoadColor2D(std::string_view file, const TextureLoadOptions &opt)
{
     return CreateFromFile2D({
        .m_File       = file,
        .m_Range      = ImageRange::LDR,
        .m_ColorSpace = ColorSpace::Auto,
        .b_FlipY      = (opt.m_FlipY == FlipY::YES),
        .b_UseCache   = (opt.m_CacheMode == TextureCacheMode::UseCache),
        .m_Sampler    = opt.m_Sampler,
        .m_DebugName  = opt.m_DebugName
    });
}

TextureHandle TextureManager::AllocateSlot()
{
    if (!m_FreeList.empty())
    {
        const u32 idx = m_FreeList.back();
        m_FreeList.pop_back();
        return TextureHandle{ idx + 1, m_Slots[idx].m_Generation };
    }

    m_Slots.push_back({});
    const u32 idx = (u32)m_Slots.size() - 1;
    return TextureHandle{ idx + 1, m_Slots[idx].m_Generation };
}

TextureManager::Slot* TextureManager::GetSlot(TextureHandle h)
{
    if (!h) return nullptr;

    const u32 idx = h.m_Id - 1;
    if (idx >= m_Slots.size()) return nullptr;

    Slot& s = m_Slots[idx];
    if (!s.b_Alive) return nullptr;
    if (s.m_Generation != h.m_Generation) return nullptr;

    return &s;
}

const TextureManager::Slot* TextureManager::GetSlot(TextureHandle h) const
{
    return const_cast<TextureManager*>(this)->GetSlot(h);
}

bool TextureManager::IsValid(TextureHandle h) const
{
    return GetSlot(h) != nullptr;
}

const TextureDesc& TextureManager::GetDesc(TextureHandle h) const
{
    const Slot* s = GetSlot(h);
    CORE_ASSERT(s, "GetDesc: invalid TextureHandle");
    return s->m_Desc;
}

// ---------------- cache helpers ----------------
TextureHandle TextureManager::TryGetFromCache(const TextureKey& key)
{
    auto it = m_FileCache.find(key);
    if (it == m_FileCache.end())
        return {};

    TextureHandle h = it->second;
    Slot* s = GetSlot(h);
    if (!s)
    {
        m_FileCache.erase(it);
        return {};
    }

    ++s->m_RefCount;
    return h;
}

TextureHandle TextureManager::StoreToCache(const TextureKey& key, TextureHandle h)
{
    Slot* s = GetSlot(h);
    CORE_ASSERT(s, "StoreToCache: invalid TextureHandle");

    if (auto it = m_FileCache.find(key); it != m_FileCache.end())
    {
        if (!GetSlot(it->second))
        {
            m_FileCache.erase(it);
        }
        else
        {
            CORE_ASSERT(it->second == h, "StoreToCache: duplicate live TextureKey");
        }
    }

    s->b_HasKey = true;
    s->m_Key = key;

    if (s->m_RefCount == 0)
        s->m_RefCount = 1;

    m_FileCache.insert_or_assign(s->m_Key, h);
    return h;
}

// ---------------- CreateEmpty ----------------
TextureHandle TextureManager::CreateEmpty(const TextureDesc& inDesc, std::string_view debugName)
{
    ValidateTextureDesc(inDesc);
    EnsureUnitCacheInit();

    GLStateGuard state(*this);
    return CreateEmptyInternal(inDesc, debugName);
}

// ---------------- CreateFromPixels2D ----------------
TextureHandle TextureManager::CreateFromPixels2D(const TextureDesc& inDesc,
                                                 std::span<const std::byte> pixels,
                                                 u32 rowStrideBytes,
                                                 std::string_view debugName)
{
    EnsureUnitCacheInit();

    TextureDesc desc = inDesc;
    desc.m_Type = TextureType::Tex2D;
    ValidateTextureDesc(desc);

    GLStateGuard state(*this);

    TextureHandle h = CreateEmptyInternal(desc, debugName);
    Slot* s = GetSlot(h);
    if (!s) return {};

    const GLFormat gf = ToGL(s->m_Desc.m_Format);
    const u32 mipLevels = (u32)s->m_Desc.m_MipLevels;

    const u64 tightRowBytes  = (u64)s->m_Desc.m_Width * (u64)gf.m_Bpp;
    const u64 actualRowBytes = (rowStrideBytes != 0) ? (u64)rowStrideBytes : tightRowBytes;

    CORE_ASSERT(actualRowBytes >= tightRowBytes, "rowStrideBytes is smaller than required row size");

    const u64 requiredBytes =
        (s->m_Desc.m_Height > 0)
        ? actualRowBytes * (u64)(s->m_Desc.m_Height - 1) + tightRowBytes
        : 0;

    CORE_ASSERT((u64)pixels.size_bytes() >= requiredBytes, "pixel buffer too small for texture upload");

    PixelStoreGuard guard;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if (rowStrideBytes != 0)
    {
        CORE_ASSERT(rowStrideBytes % gf.m_Bpp == 0, "rowStrideBytes must be a multiple of bytes-per-pixel");
        glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint)(rowStrideBytes / gf.m_Bpp));
    }
    else
    {
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    }

    glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    0,
                    0,
                    (GLsizei)s->m_Desc.m_Width,
                    (GLsizei)s->m_Desc.m_Height,
                    gf.m_DataFmt,
                    gf.m_DataType,
                    pixels.data());

    if (s->m_Desc.b_GenerateMips && mipLevels > 1)
        glGenerateMipmap(GL_TEXTURE_2D);

    return h;
}

// ---------------- CreateFromFile2D ----------------
TextureHandle TextureManager::CreateFromFile2D(const TextureFile2DLoadDesc& d)
{
    EnsureUnitCacheInit();

    if (d.m_File.empty())
        return MakeChecker(*this, d.m_Sampler);

    const std::string normFile = NormalizePath(d.m_File);

    TextureKey key{};
    key.m_Path       = normFile;
    key.b_IsCube     = false;
    key.m_Range      = d.m_Range;
    key.m_ColorSpace = d.m_ColorSpace;
    key.b_FlipY      = d.b_FlipY;
    key.m_Sampler    = d.m_Sampler;

    if (d.b_UseCache)
    {
        if (TextureHandle cached = TryGetFromCache(key))
            return cached;
    }

    if (d.m_Range == ImageRange::LDR)
    {
        int w = 0, h = 0, ch = 0;
        unsigned char* data = stbi_load(normFile.c_str(), &w, &h, &ch, 0);
        if (!data)
            return MakeChecker(*this, d.m_Sampler);

        CORE_ASSERT(ch >= 1 && ch <= 4, "Unsupported LDR image channel count");

        if (d.b_FlipY && w > 0 && h > 1)
            FlipY_U8_InPlace(data, w, h, ch);

        TextureDesc desc{};
        desc.m_Type         = TextureType::Tex2D;
        desc.m_Width        = (u32)w;
        desc.m_Height       = (u32)h;
        desc.m_Format       = ChooseLDRFormatByChannels(ch, d.m_ColorSpace);
        desc.m_Sampler      = d.m_Sampler;
        desc.m_MipLevels    = 0;
        desc.b_GenerateMips = true;

        GLStateGuard state(*this);

        TextureHandle th = CreateEmptyInternal(desc, d.m_DebugName);
        Slot* s = GetSlot(th);
        if (!s)
        {
            stbi_image_free(data);
            return {};
        }

        const GLFormat gf = ToGL(s->m_Desc.m_Format);
        const u32 mipLevels = (u32)s->m_Desc.m_MipLevels;

        PixelStoreGuard guard;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, gf.m_DataFmt, gf.m_DataType, data);

        if (s->m_Desc.b_GenerateMips && mipLevels > 1)
            glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
        return d.b_UseCache ? StoreToCache(key, th) : th;
    }

    // HDR
    {
        int w = 0, h = 0, ch = 0;
        float* data = stbi_loadf(normFile.c_str(), &w, &h, &ch, 0);
        if (!data)
            return MakeChecker(*this, d.m_Sampler);

        CORE_ASSERT(ch >= 1 && ch <= 4, "Unsupported HDR image channel count");

        if (d.b_FlipY && w > 0 && h > 1)
            FlipY_F32_InPlace(data, w, h, ch);

        TextureDesc desc{};
        desc.m_Type         = TextureType::Tex2D;
        desc.m_Width        = (u32)w;
        desc.m_Height       = (u32)h;
        desc.m_Format       = ChooseHDRFormatByChannels(ch);
        desc.m_Sampler      = d.m_Sampler;
        desc.m_MipLevels    = 0;
        desc.b_GenerateMips = true;

        GLStateGuard state(*this);

        TextureHandle th = CreateEmptyInternal(desc, d.m_DebugName);
        Slot* s = GetSlot(th);
        if (!s)
        {
            stbi_image_free(data);
            return {};
        }

        const GLFormat gf = ToGL(s->m_Desc.m_Format);
        const u32 mipLevels = (u32)s->m_Desc.m_MipLevels;

        PixelStoreGuard guard;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, gf.m_DataFmt, gf.m_DataType, data);

        if (s->m_Desc.b_GenerateMips && mipLevels > 1)
            glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
        return d.b_UseCache ? StoreToCache(key, th) : th;
    }
}

// ---------------- CreateFromFileCube ----------------
TextureHandle TextureManager::CreateFromFileCube(const TextureCubeLoadDesc& d)
{
    EnsureUnitCacheInit();

    if (d.m_BaseDir.empty() || d.m_Extension.empty())
        return {};

    const std::string normBaseDir = NormalizePath(d.m_BaseDir);
    const char* suffix[6] = { "posx", "negx", "posy", "negy", "posz", "negz" };

    auto pathOf = [&](int face) -> std::string
    {
        std::string p(normBaseDir);
        if (!p.empty() && p.back() != '/')
            p.push_back('/');

        p += suffix[face];
        p += d.m_Extension;
        return p;
    };

    TextureKey key{};
    key.m_Path       = normBaseDir + "|" + std::string(d.m_Extension);
    key.b_IsCube     = true;
    key.m_Range      = d.m_Range;
    key.m_ColorSpace = d.m_ColorSpace;
    key.b_FlipY      = false;
    key.m_Sampler    = d.m_Sampler;

    if (d.b_UseCache)
    {
        if (TextureHandle cached = TryGetFromCache(key))
            return cached;
    }

    TextureDesc desc{};
    desc.m_Type         = TextureType::Cube;
    desc.m_Sampler      = d.m_Sampler;
    desc.m_MipLevels    = 0;
    desc.b_GenerateMips = true;

    if (d.m_Range == ImageRange::LDR)
    {
        int w = 0, h = 0, ch = 0;
        unsigned char* face0 = stbi_load(pathOf(0).c_str(), &w, &h, &ch, 0);
        if (!face0)
            return {};

        CORE_ASSERT(ch >= 1 && ch <= 4, "Unsupported LDR cubemap channel count");
        CORE_ASSERT(w == h, "Cubemap face must be square");

        desc.m_Width  = (u32)w;
        desc.m_Height = (u32)h;
        desc.m_Format = ChooseLDRFormatByChannels(ch, d.m_ColorSpace);

        GLStateGuard state(*this);

        TextureHandle th = CreateEmptyInternal(desc, d.m_DebugName);
        Slot* s = GetSlot(th);
        if (!s)
        {
            stbi_image_free(face0);
            return {};
        }

        const GLFormat gf = ToGL(s->m_Desc.m_Format);
        const u32 mipLevels = (u32)s->m_Desc.m_MipLevels;

        PixelStoreGuard guard;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

        glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, 0, 0, w, h, gf.m_DataFmt, gf.m_DataType, face0);
        stbi_image_free(face0);

        for (int face = 1; face < 6; ++face)
        {
            int fw = 0, fh = 0, fch = 0;
            unsigned char* fi = stbi_load(pathOf(face).c_str(), &fw, &fh, &fch, 0);

            if (!fi || fw != w || fh != h || fch != ch)
            {
                if (fi) stbi_image_free(fi);
                Destroy(th);
                return {};
            }

            glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                            0, 0, 0, w, h, gf.m_DataFmt, gf.m_DataType, fi);
            stbi_image_free(fi);
        }

        if (s->m_Desc.b_GenerateMips && mipLevels > 1)
            glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        return d.b_UseCache ? StoreToCache(key, th) : th;
    }

    // HDR cube
    {
        int w = 0, h = 0, ch = 0;
        float* face0 = stbi_loadf(pathOf(0).c_str(), &w, &h, &ch, 0);
        if (!face0)
            return {};

        CORE_ASSERT(ch >= 1 && ch <= 4, "Unsupported HDR cubemap channel count");
        CORE_ASSERT(w == h, "Cubemap face must be square");

        desc.m_Width  = (u32)w;
        desc.m_Height = (u32)h;
        desc.m_Format = ChooseHDRFormatByChannels(ch);

        GLStateGuard state(*this);

        TextureHandle th = CreateEmptyInternal(desc, d.m_DebugName);
        Slot* s = GetSlot(th);
        if (!s)
        {
            stbi_image_free(face0);
            return {};
        }

        const GLFormat gf = ToGL(s->m_Desc.m_Format);
        const u32 mipLevels = (u32)s->m_Desc.m_MipLevels;

        PixelStoreGuard guard;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

        glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, 0, 0, w, h, gf.m_DataFmt, gf.m_DataType, face0);
        stbi_image_free(face0);

        for (int face = 1; face < 6; ++face)
        {
            int fw = 0, fh = 0, fch = 0;
            float* fi = stbi_loadf(pathOf(face).c_str(), &fw, &fh, &fch, 0);

            if (!fi || fw != w || fh != h || fch != ch)
            {
                if (fi) stbi_image_free(fi);
                Destroy(th);
                return {};
            }

            glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                            0, 0, 0, w, h, gf.m_DataFmt, gf.m_DataType, fi);
            stbi_image_free(fi);
        }

        if (s->m_Desc.b_GenerateMips && mipLevels > 1)
            glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        return d.b_UseCache ? StoreToCache(key, th) : th;
    }
}

// ---------------- Bind ----------------
void TextureManager::Bind(TextureHandle h, u32 unit) const
{
    const Slot* s = GetSlot(h);
    if (!s) return;

    EnsureUnitCacheInit();
    if (unit >= m_BoundTex.size()) return;

    if (m_BoundTex[unit] == s->m_GLTex &&
        m_BoundSampler[unit] == s->m_GLSampler)
    {
        return;
    }

    if (m_ActiveUnit != unit)
    {
        glActiveTexture(GL_TEXTURE0 + (GLenum)unit);
        m_ActiveUnit = unit;
    }

    const GLenum target = (s->m_Desc.m_Type == TextureType::Cube)
        ? GL_TEXTURE_CUBE_MAP
        : GL_TEXTURE_2D;

    if (m_BoundTex[unit] != s->m_GLTex)
        glBindTexture(target, (GLuint)s->m_GLTex);

    if (m_BoundSampler[unit] != s->m_GLSampler)
        glBindSampler((GLuint)unit, (GLuint)s->m_GLSampler);

    m_BoundTex[unit]     = s->m_GLTex;
    m_BoundSampler[unit] = s->m_GLSampler;
}

// ---------------- GenerateMips ----------------
void TextureManager::GenerateMips(TextureHandle h)
{
    Slot* s = GetSlot(h);
    if (!s) return;

    const u32 mipLevels = (u32)s->m_Desc.m_MipLevels;
    if (mipLevels <= 1) return;

    EnsureUnitCacheInit();
    GLStateGuard state(*this);

    const GLenum target = (s->m_Desc.m_Type == TextureType::Cube)
        ? GL_TEXTURE_CUBE_MAP
        : GL_TEXTURE_2D;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(target, (GLuint)s->m_GLTex);
    glGenerateMipmap(target);
}

void TextureManager::InitDefaultTextures()
{
    CreateDefaultTextures();
} 

// ---------------- Destroy ----------------
void TextureManager::Destroy(TextureHandle h)
{
    Slot* s = GetSlot(h);
    if (!s) return;

    CORE_ASSERT(s->m_RefCount > 0, "Destroy: ref-count underflow");

    if (s->m_RefCount > 1)
    {
        --s->m_RefCount;
        return;
    }

    const u32 oldTex     = s->m_GLTex;
    const u32 oldSampler = s->m_GLSampler;

    if (s->b_HasKey)
    {
        auto it = m_FileCache.find(s->m_Key);
        if (it != m_FileCache.end() && it->second == h)
            m_FileCache.erase(it);

        s->b_HasKey = false;
    }

    GLDeleteTex(s->m_GLTex);

    s->m_Desc      = TextureDesc{};
    s->m_RefCount  = 0;
    s->b_Alive     = false;
    s->m_Generation++;
    s->m_GLTex = 0;
    s->m_GLSampler = 0;
    s->m_Key = TextureKey{};
    s->b_HasKey = false;

    m_FreeList.push_back(h.m_Id - 1);

    if (!m_BoundTex.empty())
    {
        for (size_t i = 0; i < m_BoundTex.size(); ++i)
        {
            if (m_BoundTex[i] == oldTex)         m_BoundTex[i] = 0;
            if (m_BoundSampler[i] == oldSampler) m_BoundSampler[i] = 0;
        }
    }
}

TextureHandle TextureManager::CreateEmptyInternal(const TextureDesc& inDesc, std::string_view debugName)
{
    ValidateTextureDesc(inDesc);

    TextureDesc desc = inDesc;
    const u32 mipLevels = EffectiveMipLevels(desc);
    desc.m_MipLevels = (u8)mipLevels;

    const GLenum target = (desc.m_Type == TextureType::Cube)
        ? GL_TEXTURE_CUBE_MAP
        : GL_TEXTURE_2D;

    const GLFormat gf = ToGL(desc.m_Format);

    TextureHandle h = AllocateSlot();
    Slot& slot = m_Slots[h.m_Id - 1];

    GLuint tex = 0;
    glGenTextures(1, &tex);
    if (tex == 0)
    {
        slot.m_Generation++;
        m_FreeList.push_back(h.m_Id - 1);
        return {};
    }

    slot.m_GLTex     = (u32)tex;
    slot.m_GLSampler = GetOrCreateSampler(desc.m_Sampler);
    slot.m_Desc      = desc;
    slot.m_Generation = h.m_Generation;
    slot.b_Alive     = true;
    slot.m_RefCount  = 1;
    slot.b_HasKey    = false;

    SetDebugLabel(GL_TEXTURE, slot.m_GLTex, debugName);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(target, (GLuint)slot.m_GLTex);

    // OpenGL 4.3 core path: always immutable storage
    glTexStorage2D(target,
                   (GLsizei)mipLevels,
                   gf.m_InternalFmt,
                   (GLsizei)desc.m_Width,
                   (GLsizei)desc.m_Height);

    SetMipRange(target, mipLevels);

    return h;
}

TextureHandle TextureManager::LoadData2D(std::string_view file, const TextureLoadOptions &opt)
{
      return CreateFromFile2D({
        .m_File       = file,
        .m_Range      = ImageRange::LDR,
        .m_ColorSpace = ColorSpace::ForceLinear,
        .b_FlipY      = (opt.m_FlipY == FlipY::YES),
        .b_UseCache   = (opt.m_CacheMode == TextureCacheMode::UseCache),
        .m_Sampler    = opt.m_Sampler,
        .m_DebugName  = opt.m_DebugName
    });
}

TextureHandle TextureManager::LoadHDR2D(std::string_view file, const TextureLoadOptions &opt)
{
    return CreateFromFile2D({
        .m_File       = file,
        .m_Range      = ImageRange::HDR,
        .m_ColorSpace = ColorSpace::ForceLinear,
        .b_FlipY      = (opt.m_FlipY == FlipY::YES),
        .b_UseCache   = (opt.m_CacheMode == TextureCacheMode::UseCache),
        .m_Sampler    = opt.m_Sampler,
        .m_DebugName  = opt.m_DebugName
    });
}

TextureHandle TextureManager::LoadColorCube(std::string_view baseDir, std::string_view extension, const TextureLoadOptions &opt)
{
    return CreateFromFileCube({
        .m_BaseDir    = baseDir,
        .m_Extension  = extension,
        .m_Range      = ImageRange::LDR,
        .m_ColorSpace = ColorSpace::Auto,
        .b_UseCache   = (opt.m_CacheMode == TextureCacheMode::UseCache),
        .m_Sampler    = opt.m_Sampler,
        .m_DebugName  = opt.m_DebugName
    });
}

TextureHandle TextureManager::LoadHDRCube(std::string_view baseDir, std::string_view extension, const TextureLoadOptions &opt)
{
    return CreateFromFileCube({
        .m_BaseDir    = baseDir,
        .m_Extension  = extension,
        .m_Range      = ImageRange::HDR,
        .m_ColorSpace = ColorSpace::ForceLinear,
        .b_UseCache   = (opt.m_CacheMode == TextureCacheMode::UseCache),
        .m_Sampler    = opt.m_Sampler,
        .m_DebugName  = opt.m_DebugName
    });
}

TextureHandle TextureManager::LoadAlbedoMap(std::string_view file, const TextureLoadOptions &opt)
{
    return LoadColor2D(file, opt);
}

TextureHandle TextureManager::LoadNormalMap(std::string_view file, const TextureLoadOptions &opt)
{
    return LoadData2D(file, opt);
}

TextureHandle TextureManager::LoadMRMap(std::string_view file, const TextureLoadOptions &opt)
{
    return LoadData2D(file, opt);
}

TextureHandle TextureManager::LoadEmissiveMap(std::string_view file, const TextureLoadOptions &opt)
{
    return LoadColor2D(file, opt);
}

u32 TextureManager::GetNativeTexture(TextureHandle h) const
{
    const Slot* s = GetSlot(h);
    CORE_ASSERT(s, "GetNativeTexture: invalid TextureHandle");
    return s->m_GLTex;
}

TextureHandle TextureManager::GetDefaultTexture(DefaultTextureKind kind)const
{
    //CreateDefaultTextures();

    const TextureHandle h = m_DefaultTextures[(u8)kind];
    CORE_ASSERT(IsValid(h), "GetDefaultTexture: default texture is invalid");
    return h;
}

void TextureManager::CreateDefaultTextures()
{
     if (b_DefaultTexturesReady)
        return;

    // White RGBA: for Albedo / Emissive
    m_DefaultTextures[(u8)DefaultTextureKind::White] =
        CreateSolidRGBA8(255, 255, 255, 255, "Default_White");

    // Flat normal: (0.5, 0.5, 1.0)
    m_DefaultTextures[(u8)DefaultTextureKind::FlatNormal] =
        CreateSolidRGBA8(128, 128, 255, 255, "Default_FlatNormal");

    // White single-channel semantic, but using RGBA8 for simplicity
    m_DefaultTextures[(u8)DefaultTextureKind::WhiteR] =
        CreateSolidRGBA8(255, 255, 255, 255, "Default_WhiteR");

    // ORM default: AO=1, Roughness=1, Metallic=1
    m_DefaultTextures[(u8)DefaultTextureKind::WhiteORM] =
        CreateSolidRGBA8(255, 255, 255, 255, "Default_WhiteORM");

    b_DefaultTexturesReady = true;
}


TextureHandle TextureManager::CreateSolidRGBA8(u8 r, u8 g, u8 b, u8 a,
                               std::string_view debugName)
{
    const std::array<u8, 4> pixel{ r, g, b, a };

    TextureDesc desc{};
    desc.m_Type         = TextureType::Tex2D;
    desc.m_Format       = TextureFormat::RGBA8;
    desc.m_Width        = 1;
    desc.m_Height       = 1;
    desc.m_MipLevels    = 1;
    desc.b_GenerateMips = false;
    desc.m_Sampler      = SamplerDesc{};

    return CreateFromPixels2D(
        desc,
        std::span<const std::byte>(
            reinterpret_cast<const std::byte*>(pixel.data()),
            sizeof(pixel)),
        0,
        debugName
    );

}